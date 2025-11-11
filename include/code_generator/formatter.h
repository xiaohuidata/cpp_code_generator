#ifndef CODE_GENERATOR_FORMATTER_HPP
#define CODE_GENERATOR_FORMATTER_HPP

#include "zero_copy_stream.h"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/core/noncopyable.hpp>
#include <string>
#include <vector>
#include <stack>

namespace code_generator {

class Formatter : private boost::noncopyable {
public:
	enum class IndentStyle { SPACES_2 = 2, SPACES_4 = 4, TABS = -1 };

	explicit Formatter(ZeroCopyOutputStreamPtr output, 
				IndentStyle style = IndentStyle::SPACES_2,
				bool use_braces = true);
	~Formatter();

	// 基础输出
	Formatter& Print(const std::string& text);
	Formatter& Print(const char* text);
	Formatter& Print(int value);
	Formatter& Print(const std::vector<std::string>& lines);

	// Boost格式化
	template<typename... Args>
	Formatter& PrintFormat(const std::string& format, Args&&... args) {
		boost::format fmt(format);
		return Print(FormatRecursive(fmt, std::forward<Args>(args)...));
	}

	// 缩进控制
	Formatter& Indent();
	Formatter& Outdent();
	Formatter& SetIndentLevel(int level);
	int GetIndentLevel() const { return indent_level_; }

	// 作用域RAII
	class Scope {
	public:
		Scope(Formatter* formatter, const std::string& prefix = "");
		~Scope();

		// 允许移动但不允许拷贝
		Scope(Scope&& other ) = default;
		Scope& operator=(Scope&& other) = default;

		// 删除拷贝构造函数和赋值操作符
		Scope(const Scope&) = delete;
		Scope& operator=(const Scope&) = delete;

	private:
		Formatter* formatter_;
	};

	// 代码结构
	std::unique_ptr<Scope> OpenBlock(const std::string& prefix = "");
	void CloseBlock(const std::string& suffix = "");
	// 辅助方法：打开块但不返回Scope
	void OpenBlockInternal(const std::string& prefix = "");

	Formatter& EndLine();
	Formatter& AddLine(const std::string& line = "");
	Formatter& AddComment(const std::string& comment);

	// 控制结构
	Formatter& If(const std::string& condition);
	Formatter& Else();
	Formatter& ElseIf(const std::string& condition);
	Formatter& EndIf();

	Formatter& For(const std::string& loop_header);
	Formatter& While(const std::string& condition);
	Formatter& EndLoop();

	// 类型定义
	Formatter& Class(const std::string& name, const std::string& inheritance = "");
	Formatter& Struct(const std::string& name, const std::string& inheritance = "");
	Formatter& EndClass();

	Formatter& Namespace(const std::string& name);
	Formatter& EndNamespace();

	Formatter& Enum(const std::string& name, const std::vector<std::string>& values);

	// 访问控制
	Formatter& Public();
	Formatter& Private();
	Formatter& Protected();

	// 预处理指令
	Formatter& Include(const std::string& header);
	Formatter& Define(const std::string& macro);
	Formatter& IfDef(const std::string& macro);
	Formatter& IfNDef(const std::string& macro);
	Formatter& EndIfDef();

	// 工具方法
	std::string CurrentIndent() const;
	void Flush();

private:
	ZeroCopyOutputStreamPtr output_;
	IndentStyle indent_style_;
	bool use_braces_;
	int indent_level_;
	bool at_start_of_line_;

	enum class BlockType { NONE, IF, ELSE, FOR, WHILE, CLASS, STRUCT, NAMESPACE };
	struct BlockState {
		BlockType type;
		std::string prefix;
	};
	std::stack<BlockState> block_stack_;

	void WriteIndent();
	void WriteString(const std::string& str);

	// Boost格式化递归辅助
	template<typename T, typename... Args>
	std::string FormatRecursive(boost::format& fmt, T&& value, Args&&... args) {
		fmt % std::forward<T>(value);
		return FormatRecursive(fmt, std::forward<Args>(args)...);
	}

	std::string FormatRecursive(boost::format& fmt) {
		return fmt.str();
	}

};

} // namespace code_generator

#endif
