// cpp_generator.h
#ifndef CPP_GENERATOR_H
#define CPP_GENERATOR_H

#include "formatter.h"
#include <string>
#include <vector>
#include <map>

namespace code_generator{

// C++代码生成配置
struct CppGeneratorOptions {
    Formatter::IndentStyle indent_style = Formatter::IndentStyle::SPACES_2;
    bool use_braces = true;
    bool generate_comments = true;
    std::string file_header_comment;
    bool use_pragma_once = true;
    bool use_include_guards = false;
    std::string include_guard_prefix;
};

// C++类型信息
struct CppType {
    std::string name;
    bool is_const = false;
    bool is_reference = false;
    bool is_pointer = false;
    
    std::string ToString() const;
};

// 函数参数
struct CppParameter {
    CppType type;
    std::string name;
    std::string default_value;
    
    std::string ToString() const;
};

// 函数信息
struct CppFunction {
    std::string return_type;
    std::string name;
    std::vector<CppParameter> parameters;
    bool is_virtual = false;
    bool is_pure_virtual = false;
    bool is_const = false;
    bool is_static = false;
    std::string body;
    std::string access_specifier = "public";
    
    std::string GetSignature() const;
};

// 类成员变量
struct CppMember {
    CppType type;
    std::string name;
    std::string initializer;
    std::string access_specifier = "private";
    
    std::string ToString() const;
};

// 类信息
struct CppClass {
    std::string name;
    std::vector<std::string> base_classes;
    std::vector<CppMember> members;
    std::vector<CppFunction> functions;
    std::vector<std::string> forward_declarations;
    
    void AddFunction(const CppFunction& func);
    void AddMember(const CppMember& member);
};

// C++代码生成器
class CppGenerator {
public:
    explicit CppGenerator(ZeroCopyOutputStreamPtr output, 
                         const CppGeneratorOptions& options = CppGeneratorOptions());
    
    // 文件控制
    void BeginFile(const std::string& filename, const std::vector<std::string>& includes = {});
    void EndFile();
    
    // 命名空间
    void BeginNamespace(const std::string& name);
    void EndNamespace();
    
    // 类生成
    void GenerateClass(const CppClass& cls);
    void GenerateClassDeclaration(const CppClass& cls);
    void GenerateClassImplementation(const CppClass& cls, const std::string& namespace_prefix = "");
    
    // 函数生成
    void GenerateFunction(const CppFunction& func, bool in_class = false);
    void GenerateFunctionDeclaration(const CppFunction& func);
    void GenerateFunctionImplementation(const CppFunction& func, const std::string& class_name = "");
    
    // 枚举生成
    void GenerateEnum(const std::string& name, const std::vector<std::string>& values, 
                     const std::string& type = "");
    
    // 访问器生成
    void GenerateGetter(const CppMember& member);
    void GenerateSetter(const CppMember& member);
    
    // 注释生成
    void GenerateFileHeader(const std::string& filename);
    void GenerateFunctionComment(const CppFunction& func);
    void GenerateMemberComment(const CppMember& member);
    
    Formatter& GetFormatter() { return formatter_; }

private:
    Formatter formatter_;
    CppGeneratorOptions options_;
    std::string current_filename_;
    
    void GenerateIncludeGuards(bool begin);
    void GenerateIncludes(const std::vector<std::string>& includes);
    std::string BuildIncludeGuard(const std::string& filename);
};

} // namespace code_generator
#endif
