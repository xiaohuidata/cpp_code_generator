// enhanced_cpp_generator.cpp
#include "code_generator/enhanced_cpp_generator.h"
#include "code_generator/stream_adapters.h"
#include <fstream>
#include <sstream>
#include <boost/filesystem.hpp>

namespace code_generator{

EnhancedCppGenerator::EnhancedCppGenerator(const std::string& output_dir)
    : output_dir_(output_dir) {
    // 创建输出目录
    EnsureDirectory(output_dir_);
}

bool EnhancedCppGenerator::GenerateFromConfig(const code_generator::CodeGenConfig::ProjectConfig& config) {
    // 设置输出目录
    if (!config.output_dir.empty()) {
        output_dir_ = config.output_dir;
        EnsureDirectory(output_dir_);
    }
    
    // 生成所有文件
    for (const auto& file_config : config.files) {
        if (!GenerateFile(file_config)) {
            return false;
        }
    }
    
    // 处理文件复制
    for (const auto& file_config : config.files) {
        for (const auto& copy_file : file_config.copy_files) {
            std::string source = copy_file;
            //std::string destination = output_dir_ + "/" + std::filesystem::path(copy_file).filename().string();
            std::string destination = output_dir_ + "/" + boost::filesystem::path(copy_file).filename().string();
            
            if (!CopyFile(source, destination)) {
                // 如果直接复制失败，尝试解析为代码库引用
                std::string resolved_code = ResolveCodeReference(copy_file);
                if (!resolved_code.empty()) {
                    std::ofstream out_file(destination);
                    if (out_file) {
                        out_file << resolved_code;
                    }
                }
            }
        }
        
        // 处理代码片段插入
        for (const auto& snippet_ref : file_config.insert_snippets) {
            std::string file_path = output_dir_ + "/" + file_config.filename;
            std::string snippet = ResolveCodeReference(snippet_ref);
            if (!snippet.empty()) {
                InsertSnippet(file_path, snippet);
            }
        }
    }
    
    return true;
}

bool EnhancedCppGenerator::GenerateFromConfigFile(const std::string& config_file) {
    if (!config_parser_) {
        config_parser_ = std::make_shared<code_generator::ConfigParser>();
    }
    
    if (!config_parser_->LoadFromFile(config_file)) {
        return false;
    }
    
    return GenerateFromConfig(config_parser_->GetProjectConfig());
}

bool EnhancedCppGenerator::GenerateFile(const code_generator::CodeGenConfig::FileConfig& file_config) {
    std::string file_path = output_dir_ + "/" + file_config.filename;
    
    // 创建文件输出流
    std::ofstream file_stream(file_path);
    if (!file_stream) {
        return false;
    }
    
    // 创建字符串输出流
    //std::stringstream buffer;
    std::string buffer;
    code_generator::ZeroCopyOutputStreamPtr string_output(new code_generator::StringOutputStream(&buffer));
    
    // 创建格式化器
    CppGeneratorOptions options;
    options.indent_style = code_generator::Formatter::IndentStyle::SPACES_2;
    options.use_pragma_once = true;
    options.generate_comments = true;
    
    CppGenerator generator(string_output, options);
    
    // 开始文件
    std::vector<std::string> includes = file_config.includes;
    
    // 添加公共包含
    if (config_parser_) {
        const auto& project_config = config_parser_->GetProjectConfig();
        includes.insert(includes.end(), project_config.common_includes.begin(), project_config.common_includes.end());
    }
    
    generator.BeginFile(file_config.filename, includes);
    
    // 处理命名空间
    std::vector<std::string> current_namespaces;
    for (const auto& ns : file_config.namespaces) {
        generator.BeginNamespace(ns);
        current_namespaces.push_back(ns);
    }
    
    // 生成全局函数
    for (const auto& func_config : file_config.functions) {
        if (!GenerateFunction(func_config, generator.GetFormatter(), false)) {
            return false;
        }
    }
    
    // 生成全局变量
    for (const auto& global_config : file_config.globals) {
        if (!GenerateGlobal(global_config, generator.GetFormatter())) {
            return false;
        }
    }
    
    // 生成类
    for (const auto& class_config : file_config.classes) {
        if (!GenerateClass(class_config, generator.GetFormatter())) {
            return false;
        }
    }
    
    // 结束命名空间
    for (size_t i = 0; i < current_namespaces.size(); ++i) {
        generator.EndNamespace();
    }
    
    // 结束文件
    generator.EndFile();
    
    // 写入文件
    //file_stream << buffer.str();
    file_stream << buffer;
    file_stream.close();
    
    return true;
}

bool EnhancedCppGenerator::GenerateClass(const code_generator::CodeGenConfig::ClassConfig& class_config, code_generator::Formatter& formatter) {
    CppClass cpp_class = ConvertToCppClass(class_config);
    
    // 使用CppGenerator生成类声明
    //std::stringstream buffer;
    std::string buffer;
    code_generator::ZeroCopyOutputStreamPtr string_output(new code_generator::StringOutputStream(&buffer));
    
    CppGeneratorOptions options;
    options.indent_style = code_generator::Formatter::IndentStyle::SPACES_2;
    CppGenerator generator(string_output, options);
    
    generator.GenerateClassDeclaration(cpp_class);
    
    // 将生成的代码写入主格式化器
    //std::string class_code = buffer.str();
    std::string class_code = buffer;
    std::vector<std::string> lines;
    
    size_t start = 0;
    size_t end = class_code.find('\n');
    while (end != std::string::npos) {
        lines.push_back(class_code.substr(start, end - start));
        start = end + 1;
        end = class_code.find('\n', start);
    }
    
    formatter.Print(lines);
    
    return true;
}

bool EnhancedCppGenerator::GenerateFunction(const code_generator::CodeGenConfig::FunctionConfig& func_config, code_generator::Formatter& formatter, bool in_class) {
    CppFunction cpp_function = ConvertToCppFunction(func_config);
    
    if (in_class) {
        // 在类中生成函数声明
        if (config_parser_ && config_parser_->GetProjectConfig().code_templates.count("function_comment")) {
            std::string comment = config_parser_->ApplyTemplate("function_comment", {{"function_name", func_config.name}});
            formatter.AddComment(comment);
        }
        
        formatter.AddLine(cpp_function.GetSignature() + ";");
    } else {
        // 生成独立函数实现
        //std::stringstream buffer;
        std::string buffer;
        code_generator::ZeroCopyOutputStreamPtr string_output(new code_generator::StringOutputStream(&buffer));
        
        CppGeneratorOptions options;
        options.indent_style = code_generator::Formatter::IndentStyle::SPACES_2;
        CppGenerator generator(string_output, options);
        
        generator.GenerateFunctionImplementation(cpp_function);
        
        //std::string function_code = buffer.str();
        std::string function_code = buffer;
        std::vector<std::string> lines;
        
        size_t start = 0;
        size_t end = function_code.find('\n');
        while (end != std::string::npos) {
            lines.push_back(function_code.substr(start, end - start));
            start = end + 1;
            end = function_code.find('\n', start);
        }
        
        formatter.Print(lines);
    }
    
    return true;
}

bool EnhancedCppGenerator::GenerateMember(const code_generator::CodeGenConfig::MemberConfig& member_config, code_generator::Formatter& formatter) {
    CppMember cpp_member = ConvertToCppMember(member_config);
    
    if (!member_config.comment.empty()) {
        formatter.AddComment(member_config.comment);
    }
    
    formatter.AddLine(cpp_member.ToString());
    return true;
}

bool EnhancedCppGenerator::GenerateGlobal(const code_generator::CodeGenConfig::MemberConfig& global_config, code_generator::Formatter& formatter) {
    if (!global_config.comment.empty()) {
        formatter.AddComment(global_config.comment);
    }
    
    std::string line = global_config.type + " " + global_config.name;
    if (!global_config.initializer.empty()) {
        line += " = " + global_config.initializer;
    }
    line += ";";
    
    formatter.AddLine(line);
    return true;
}

CppClass EnhancedCppGenerator::ConvertToCppClass(const code_generator::CodeGenConfig::ClassConfig& config) {
    CppClass cpp_class;
    cpp_class.name = config.name;
    cpp_class.base_classes = config.base_classes;
    
    // 这里可以添加从模板生成成员和函数的逻辑
    for (const auto& template_name : config.templates) {
        std::string template_code = ApplyTemplate(template_name);
        // 解析模板代码并添加到类中
        // 这里需要实现模板解析逻辑
    }
    
    return cpp_class;
}

CppFunction EnhancedCppGenerator::ConvertToCppFunction(const code_generator::CodeGenConfig::FunctionConfig& config) {
    CppFunction cpp_function;
    cpp_function.name = config.name;
    cpp_function.return_type = config.return_type;
    cpp_function.is_virtual = config.is_virtual;
    cpp_function.is_pure_virtual = config.is_pure_virtual;
    cpp_function.is_const = config.is_const;
    cpp_function.is_static = config.is_static;
    cpp_function.access_specifier = config.access;
    cpp_function.body = ProcessCodeBody(config.body);
    
    //for (const auto& [type, name] : config.parameters) {
    for (const auto& iter : config.parameters) {
        const std::string& type = iter.first;
        const std::string& name = iter.second;
        CppParameter param;
        param.type.name = type;
        param.name = name;
        cpp_function.parameters.push_back(param);
    }
    
    return cpp_function;
}

CppMember EnhancedCppGenerator::ConvertToCppMember(const code_generator::CodeGenConfig::MemberConfig& config) {
    CppMember cpp_member;
    cpp_member.type.name = config.type;
    cpp_member.name = config.name;
    cpp_member.initializer = config.initializer;
    return cpp_member;
}

void EnhancedCppGenerator::RegisterTemplate(const std::string& name, const std::string& content) {
    custom_templates_[name] = content;
}

std::string EnhancedCppGenerator::ApplyTemplate(const std::string& template_name, const std::map<std::string, std::string>& variables) {
    // 首先查找自定义模板
    auto custom_it = custom_templates_.find(template_name);
    if (custom_it != custom_templates_.end()) {
        std::string result = custom_it->second;
        
        // 替换变量
        //for (const auto& [key, value] : variables) {
        for (const auto& iter : variables) {
            const std::string& key = iter.first;
            const std::string& value = iter.second;
            std::string placeholder = "${" + key + "}";
            size_t pos = 0;
            while ((pos = result.find(placeholder, pos)) != std::string::npos) {
                result.replace(pos, placeholder.length(), value);
                pos += value.length();
            }
        }
        
        return result;
    }
    
    // 然后查找配置中的模板
    if (config_parser_) {
        return config_parser_->GetTemplate(template_name);
    }
    
    return "";
}

bool EnhancedCppGenerator::CopyFile(const std::string& source, const std::string& destination) {
    try {
        //std::filesystem::copy_file(source, destination, std::filesystem::copy_options::overwrite_existing);
        boost::filesystem::copy_file(source, destination, boost::filesystem::copy_options::overwrite_existing);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool EnhancedCppGenerator::InsertSnippet(const std::string& file_path, const std::string& snippet) {
    try {
        std::fstream file(file_path, std::ios::in | std::ios::out);
        if (!file) {
            return false;
        }
        
        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string content = buffer.str();
        
        // 在文件末尾插入代码片段
        content += "\n// Inserted snippet\n";
        content += snippet;
        content += "\n// End of inserted snippet\n";
        
        file.close();
        file.open(file_path, std::ios::out | std::ios::trunc);
        file << content;
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool EnhancedCppGenerator::EnsureDirectory(const std::string& path) {
    try {
        //std::filesystem::create_directories(path);
        boost::filesystem::create_directories(path);
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void EnhancedCppGenerator::AddCodeLibrary(const std::string& name, const std::string& path) {
    code_libraries_[name] = path;
}

std::string EnhancedCppGenerator::ResolveCodeReference(const std::string& reference) {
    // 检查是否是代码库引用格式: library::component
    size_t pos = reference.find("::");
    if (pos != std::string::npos) {
        std::string library = reference.substr(0, pos);
        std::string component = reference.substr(pos + 2);
        
        auto it = code_libraries_.find(library);
        if (it != code_libraries_.end()) {
            std::string library_path = it->second;
            std::string file_path = library_path + "/" + component;
            
            // 尝试读取文件
            std::ifstream file(file_path);
            if (file) {
                std::stringstream buffer;
                buffer << file.rdbuf();
                return buffer.str();
            }
        }
    }
    
    // 如果不是代码库引用，尝试作为模板处理
    return ApplyTemplate(reference);
}

std::string EnhancedCppGenerator::ProcessCodeBody(const std::string& body) {
    std::string processed = body;
    
    // 解析关键字和引用
    processed = ResolveKeywords(processed);
    
    // 变量替换
    if (config_parser_) {
        processed = config_parser_->ReplaceVariables(processed);
    }
    
    return processed;
}

std::string EnhancedCppGenerator::ResolveKeywords(const std::string& text) {
    std::string result = text;
    
    // 处理特殊关键字
    // 例如: @include(library::component)
    size_t start_pos = 0;
    while ((start_pos = result.find("@include(", start_pos)) != std::string::npos) {
        size_t end_pos = result.find(")", start_pos);
        if (end_pos == std::string::npos) break;
        
        std::string reference = result.substr(start_pos + 9, end_pos - start_pos - 9);
        std::string resolved_code = ResolveCodeReference(reference);
        
        result.replace(start_pos, end_pos - start_pos + 1, resolved_code);
        start_pos += resolved_code.length();
    }
    
    return result;
}

}
