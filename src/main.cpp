// src/main.cpp - 简化版主程序
#include "code_generator.h"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

void TestBasicFormatter() {
    std::cout << "=== 测试基础格式化器 ===" << std::endl;
    
    //code_generator::FileOutputStream file_output("formatted_output.cpp");
    code_generator::ZeroCopyOutputStreamPtr output(new code_generator::FileOutputStream("formatted_output.cpp"));
    code_generator::Formatter formatter(
        code_generator::ZeroCopyOutputStreamPtr(output), 
        code_generator::Formatter::IndentStyle::SPACES_4
    );
    
    formatter.AddComment("这是一个测试文件");
    formatter.Include("<iostream>");
    formatter.Include("<string>");
    formatter.EndLine();
    
    formatter.Namespace("my_namespace");
    
    // 使用手动作用域管理
    formatter.Class("MyClass");
    formatter.Public();
    formatter.AddLine("MyClass();");
    formatter.AddLine("~MyClass();");
    formatter.EndLine();
    formatter.AddLine("void DoSomething(int value);");
    formatter.Private();
    formatter.AddLine("int data_;");
    formatter.EndClass();
    
    formatter.EndNamespace();
    
    std::cout << "基础格式化测试完成" << std::endl;
}

void TestConditionalFormatting() {
    std::cout << "\n=== 测试条件语句格式化 ===" << std::endl;
    
    //code_generator::FileOutputStream file_output("conditional_output.cpp");
    code_generator::ZeroCopyOutputStreamPtr output(new code_generator::FileOutputStream("conditional_output.cpp"));
    code_generator::Formatter formatter(
        code_generator::ZeroCopyOutputStreamPtr(output), 
        code_generator::Formatter::IndentStyle::SPACES_2
    );
    
    formatter.AddComment("条件语句测试");
    
    // 测试 if-else 语句
    formatter.If("x > 0");
    formatter.AddLine("std::cout << \"Positive\" << std::endl;");
    formatter.ElseIf("x < 0");
    formatter.AddLine("std::cout << \"Negative\" << std::endl;");
    formatter.Else();
    formatter.AddLine("std::cout << \"Zero\" << std::endl;");
    formatter.EndIf();
    
    // 测试循环
    formatter.For("int i = 0; i < 10; ++i");
    formatter.AddLine("std::cout << i << std::endl;");
    formatter.EndLoop();
    
    std::cout << "条件语句测试完成" << std::endl;
}

void TestOpenBlockUsage() {
    std::cout << "\n=== 测试 OpenBlock 使用 ===" << std::endl;
    
    //code_generator::FileOutputStream file_output("openblock_output.cpp");
    code_generator::ZeroCopyOutputStreamPtr output(new code_generator::FileOutputStream("openblock_output.cpp"));
    code_generator::Formatter formatter(
        code_generator::ZeroCopyOutputStreamPtr(output), 
        code_generator::Formatter::IndentStyle::SPACES_2
    );
    
    // 正确的 OpenBlock 使用方式
    {
        auto scope = formatter.OpenBlock("void TestFunction()");
        formatter.AddLine("// 函数体内容");
        formatter.AddLine("return 0;");
    } // scope 在这里自动销毁，调用 Outdent
    {
        //auto scope = formatter.OpenBlock("void TestFunction()");
        formatter.OpenBlockInternal("void TestInternalFunction()");
        formatter.AddLine("// 函数体内容");
        formatter.AddLine("return 0;");
        formatter.CloseBlock();
    }
    
    std::cout << "OpenBlock 测试完成" << std::endl;
}


int main(int argc, char* argv[]) {
    try {
        po::options_description desc("C++ Code Generator Options");
        desc.add_options()
            ("help,h", "Show help message")
            ("version,v", "Show version information")
            ("test", "Run tests")
            ("config,c", po::value<std::string>(), "Configuration file")
            ("output,o", po::value<std::string>()->default_value("./generated"), "Output directory")
            ("template,t", po::value<std::string>(), "Template name")
            ("list-templates,l", "List available templates")
            ("verbose", "Verbose output");

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 0;
        }

        if (vm.count("version")) {
            std::cout << "C++ Code Generator " << code_generator::GetVersion() << std::endl;
            std::cout << "Use --help for usage information." << std::endl;
            std::cout << "Use --test to run tests." << std::endl;
            return 0;
        }

        if (vm.count("test")) {
            TestBasicFormatter();
            TestConditionalFormatting();
            TestOpenBlockUsage();
            std::cout << "\n所有测试完成！" << std::endl;
            return 0;
        }

        if (vm.count("list-templates")) {
            // 列出可用模板
            std::cout << "Available templates:" << std::endl;
            std::cout << "  - singleton" << std::endl;
            std::cout << "  - factory" << std::endl;
            std::cout << "  - observer" << std::endl;
            return 0;
        }

        // 这里可以添加主要的代码生成逻辑

        std::cout << "C++ Code Generator completed successfully!" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

namespace code_generator {

std::string GetVersion() {
    return "1.0.0";
}

std::string GetBuildInfo() {
    return "Built with Boost C++ Libraries";
}

} // namespace code_generator
