#include "CgroupManager.hpp"

#include <filesystem>
#include <sys/resource.h>
#include <unistd.h>

#include <iostream>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <fstream>

namespace fs = std::filesystem;

namespace {

bool writeValue(const std::string& path, const std::string& value)
{
    std::ofstream out(path.c_str(), std::ios::out | std::ios::binary);
    if (!out.is_open())
    {
        return false;
    }
    out << value;
    return out.good();
}

bool useStrictMode()
{
    const char* strict = std::getenv("XDOJ_CGROUP_STRICT");
    return strict != nullptr && std::string(strict) == "1";
}

}

bool CgroupManager::apply(int timeLimitMs, std::int64_t memoryLimitBytes, std::string& error, std::string language) const
{
    error.clear();
    const bool isJava = (language == "Java");

    const char* cgroupRoot = std::getenv("XDOJ_CGROUP_ROOT");

    if (cgroupRoot != nullptr && *cgroupRoot != '\0')
    {
        const std::string root(cgroupRoot);
        const std::string cgPath = (fs::path(root) / ("judge-" + std::to_string(getpid()))).string();
        std::error_code ec;
        fs::create_directories(cgPath, ec);
        if (!ec)
        {
            bool ok = true;
            // Java 进程需要给 JVM 预留一部分非堆内存，不能把 cgroup 限制写成固定的 1024 字节。
            // 这里统一按传入限制写入，具体堆大小由 SandboxRuntime::tuneJavaMemoryIfNeeded 控制。
            ok = ok && writeValue((fs::path(cgPath) / "memory.max").string(), std::to_string(memoryLimitBytes));

            ok = ok && writeValue((fs::path(cgPath) / "pids.max").string(), "64");
            ok = ok && writeValue((fs::path(cgPath) / "cpu.max").string(), "100000 100000");
            ok = ok && writeValue((fs::path(cgPath) / "cgroup.procs").string(), std::to_string(getpid()));
            
            if (!ok && useStrictMode())
            {
                error = "写入 cgroup 限制失败。";
                return false;
            }
        }
        else if (useStrictMode())
        {
            error = "创建 cgroup 失败: " + ec.message();
            return false;
        }
    }
    else if (useStrictMode())
    {
        error = "未配置 XDOJ_CGROUP_ROOT，严格模式下拒绝运行。";
        return false;
    }

    // Java 的地址空间限制已经在 JVM 参数和 cgroup 中控制，额外套 RLIMIT_AS
    // 很容易在 JVM 启动阶段因预留虚拟内存而异常退出。
    if (memoryLimitBytes > 0 && !isJava)
    {
        struct rlimit memoryLimit;
        memoryLimit.rlim_cur = static_cast<rlim_t>(memoryLimitBytes);
        memoryLimit.rlim_max = static_cast<rlim_t>(memoryLimitBytes);
        if (setrlimit(RLIMIT_AS, &memoryLimit) != 0)
        {
            error = "设置内存限制失败。";
            return false;
        }
    }

    if (timeLimitMs > 0)
    {
        struct rlimit cpuLimit;
        const rlim_t seconds = static_cast<rlim_t>(std::max(1.0, std::ceil(timeLimitMs / 1000.0)));
        cpuLimit.rlim_cur = seconds;
        cpuLimit.rlim_max = seconds + 1;
        if (setrlimit(RLIMIT_CPU, &cpuLimit) != 0)
        {
            error = "设置 CPU 限制失败。";
            return false;
        }
    }

    return true;
}
