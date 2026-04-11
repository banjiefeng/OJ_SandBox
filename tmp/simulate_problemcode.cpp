#include "CodeSandbox/service/SandboxService.hpp"

#include <jsoncpp/json/json.h>

#include <iostream>

int main()
{
    Json::Value task;
    task["SubmitId"] = "debug-java-hello";
    task["ProblemId"] = "1";
    task["JudgeNum"] = 1;
    task["Code"] =
        "public class Main {\n"
        "    public static void main(String[] args) {\n"
        "        System.out.println(\"hello, world\");\n"
        "    }\n"
        "}";
    task["Language"] = "Java";
    task["TimeLimit"] = 1000;
    task["MemoryLimit"] = 32;

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";

    Json::Value result = SandboxService::GetInstance()->submitTask(task);
    std::cout << Json::writeString(builder, result) << std::endl;
    return 0;
}
