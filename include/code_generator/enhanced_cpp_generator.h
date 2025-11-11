#ifndef ENHANCED_CPP_GENERATOR_H
#define ENHANCED_CPP_GENERATOR_H

#include "cpp_generator.h"
#include "config_parser.h"
#include <filesystem>
#include <unordered_set>

namespace code_generator{

class EnhancedCppGenerator {
public:
    EnhancedCppGenerator(const std::string& output_dir = "./generated");
    
    // 从配置生成代码
    bool GenerateFromConfig(const code_generator::CodeGenConfig::ProjectConfig& config);
    bool GenerateFromConfigFile(const std::string& config_file);
    
    // 单个文件生成
    bool GenerateFile(const code_generator::CodeGenConfig::FileConfig& file_config);
    
    // 代码模板处理
    void RegisterTemplate(const std::string& name, const std::string& content);
    std::string ApplyTemplate(const std::string& template_name, 
                            const std::map<std::string, std::string>& variables = {});
    
    // 文件操作
    bool CopyFile(const std::string& source, const std::string& destination);
    bool InsertSnippet(const std::string& file_path, const std::string& snippet);
    bool EnsureDirectory(const std::string& path);
    
    // 关键字处理
    void AddCodeLibrary(const std::string& name, const std::string& path);
    std::string ResolveCodeReference(const std::string& reference);
    
    // 设置配置解析器
    void SetConfigParser(std::shared_ptr<code_generator::ConfigParser> parser) { config_parser_ = parser; }

private:
    std::string output_dir_;
    std::shared_ptr<code_generator::ConfigParser> config_parser_;
    std::map<std::string, std::string> custom_templates_;
    std::map<std::string, std::string> code_libraries_;
    
    // 生成具体内容
    bool GenerateClass(const code_generator::CodeGenConfig::ClassConfig& class_config, code_generator::Formatter& formatter);
    bool GenerateFunction(const code_generator::CodeGenConfig::FunctionConfig& func_config, code_generator::Formatter& formatter, bool in_class = false);
    bool GenerateMember(const code_generator::CodeGenConfig::MemberConfig& member_config, code_generator::Formatter& formatter);
    bool GenerateGlobal(const code_generator::CodeGenConfig::MemberConfig& global_config, code_generator::Formatter& formatter);
    
    // 工具方法
    CppClass ConvertToCppClass(const code_generator::CodeGenConfig::ClassConfig& config);
    CppFunction ConvertToCppFunction(const code_generator::CodeGenConfig::FunctionConfig& config);
    CppMember ConvertToCppMember(const code_generator::CodeGenConfig::MemberConfig& config);
    
    std::string ProcessCodeBody(const std::string& body);
    std::string ResolveKeywords(const std::string& text);
};

}
#endif
