/*
 * @Author: Zhang Bokai <codingzhang@126.com>
 * @Date: 2025-01-05
 * @Description: 语义分析器的定义，负责类型检查和语义错误检测
 */
#ifndef COLLIE_SEMANTIC_ANALYZER_H
#define COLLIE_SEMANTIC_ANALYZER_H

#include <memory>
#include <string>
#include <vector>
#include "semantic_common.h"
#include "symbol_table.h"
#include "../parser/ast.h"
#include "../lexer/token.h"

namespace collie {

/**
 * @brief 语义分析器类，负责类型检查和语义错误检测
 */
class SemanticAnalyzer : public ExprVisitor, public StmtVisitor, public TypeVisitor {
public:
    SemanticAnalyzer() = default;

    /**
     * @brief 分析AST，执行语义检查
     * @param statements AST根节点列表
     */
    void analyze(const std::vector<std::unique_ptr<Stmt>>& statements);

    /**
     * @brief 设置词法分析器生成的 token 序列
     * @param tokens token 序列
     */
    void set_tokens(const std::vector<Token>& tokens) {
        tokens_ = tokens;
        current_token_index_ = 0;
    }

    /**
     * @brief 获取分析过程中收集的错误
     * @return 错误列表的常量引用
     */
    const std::vector<SemanticError>& get_errors() const { return errors_; }

    /**
     * @brief 检查是否有错误
     * @return 如果有错误返回true，否则返回false
     */
    bool has_errors() const { return !errors_.empty(); }

private:
    // -----------------------------------------------------------------------------
    // 访问者模式实现
    // -----------------------------------------------------------------------------
    // ExprVisitor 接口实现
    void visitLiteral(const LiteralExpr& expr) override;
    void visitIdentifier(const IdentifierExpr& expr) override;
    void visitBinary(const BinaryExpr& expr) override;
    void visitUnary(const UnaryExpr& expr) override;
    void visitCall(const CallExpr& expr) override;
    void visitAssign(const AssignExpr& expr) override;
    void visitTuple(const TupleExpr& expr) override;
    void visitTupleMember(const TupleMemberExpr& expr) override;

    // StmtVisitor 接口实现
    void visitVarDecl(const VarDeclStmt& stmt) override;
    void visitBlock(const BlockStmt& stmt) override;
    void visitIf(const IfStmt& stmt) override;
    void visitWhile(const WhileStmt& stmt) override;
    void visitFor(const ForStmt& stmt) override;
    void visitFunction(const FunctionStmt& stmt) override;
    void visitReturn(const ReturnStmt& stmt) override;

    /**
     * @brief 访问 break 语句
     * @param stmt break 语句节点
     * @throw SemanticError 当在循环外使用 break 时
     */
    void visitBreak(const BreakStmt& stmt) override;

    /**
     * @brief 访问 continue 语句
     * @param stmt continue 语句节点
     * @throw SemanticError 当在循环外使用 continue 时
     */
    void visitContinue(const ContinueStmt& stmt) override;

    void visitExpression(const ExpressionStmt& stmt) override;

    void visitClass(const ClassStmt& stmt) override;

    // TypeVisitor 接口实现
    void visitBasicType(const BasicType& type) override;
    void visitArrayType(const ArrayType& type) override;
    void visitTupleType(const TupleType& type) override;

    // -----------------------------------------------------------------------------
    // 错误处理相关方法
    // -----------------------------------------------------------------------------

    /**
     * @brief 记录一个错误
     * @param error 错误信息
     */
    void record_error(const SemanticError& error);

    /**
     * @brief 进入错误恢复模式
     */
    void enter_panic_mode();

    /**
     * @brief 退出错误恢复模式
     */
    void exit_panic_mode();

    /**
     * @brief 同步到下一个安全点
     * 跳过当前错误的语句，直到找到一个可以继续分析的位置
     */
    void synchronize();

    /**
     * @brief 检查当前 token 是否是同步点
     * @return 如果是同步点返回 true
     */
    bool is_synchronization_point() const;

    // -----------------------------------------------------------------------------
    // 类型检查辅助方法
    // -----------------------------------------------------------------------------
    /**
     * @brief 检查类型是否是数值类型
     * @param type 要检查的类型
     * @return 如果是数值类型返回 true
     */
    bool is_numeric_type(TokenType type) const;

    /**
     * @brief 检查类型是否可以转换为数值类型
     * @param type 要检查的类型
     * @return 如果可以转换为数值类型返回 true
     */
    bool is_numeric_convertible(TokenType type) const;

    /**
     * @brief 检查类型是否可以转换为字符串类型
     * @param type 要检查的类型
     * @return 如果可以转换为字符串类型返回 true
     */
    bool is_string_convertible(TokenType type) const;

    /**
     * @brief 检查类型是否是位运算类型
     * @param type 要检查的类型
     * @return 如果是位运算类型返回 true
     */
    bool is_bit_type(TokenType type) const;

    /**
     * @brief 检查类型是否可以进行字符串连接操作
     * @param type 要检查的类型
     * @return 如果类型可以进行字符串连接则返回 true
     */
    bool is_string_concatenable(TokenType type) const;

    /**
     * @brief 检查类型是否支持比较运算
     * @param type 要检查的类型
     * @return 如果类型支持比较运算则返回 true
     */
    bool is_ordered_type(TokenType type) const;
    bool is_compatible_type(TokenType expected, TokenType actual) const;
    bool can_implicit_convert(TokenType from, TokenType to) const;

    /**
     * @brief 获取两个类型的共同类型
     * @param t1 第一个类型
     * @param t2 第二个类型
     * @return 共同类型，如果不存在则返回 INVALID
     */
    TokenType common_type(TokenType t1, TokenType t2) const;
    TokenType check_type(const Expr& expr);
    void ensure_boolean(const Expr& expr, const std::string& context);

    // -----------------------------------------------------------------------------
    // 函数重载相关方法
    // -----------------------------------------------------------------------------

    /**
     * @brief 检查函数签名是否相同
     * @param func1 已定义的函数符号
     * @param func2 新的函数声明
     * @return 如果签名相同返回 true
     */
    bool is_same_signature(const Symbol& func1, const FunctionStmt& func2);

    /**
     * @brief 查找最匹配的重载函数
     * @param overloads 所有可能的重载函数
     * @param arg_types 实际参数类型列表
     * @param error_location 错误位置信息
     * @return 最匹配的函数符号指针，如果没有匹配返回 nullptr
     */
    Symbol* find_best_overload(
        const std::vector<Symbol*>& overloads,
        const std::vector<TokenType>& arg_types,
        const Token& error_location);

    /**
     * @brief 计算函数重载的匹配得分
     * @param func 函数符号
     * @param arg_types 实际参数类型列表
     * @return 匹配得分，-1 表示不匹配
     */
    int calculate_overload_score(
        const Symbol& func,
        const std::vector<TokenType>& arg_types);

    // -----------------------------------------------------------------------------
    // Token 处理相关方法
    // -----------------------------------------------------------------------------
    const Token& current_token() const;
    const Token& previous_token() const;
    const Token& peek_next() const;
    void advance_token();

    // -----------------------------------------------------------------------------
    // 私有辅助方法
    // -----------------------------------------------------------------------------
    void reset_state();
    bool in_loop() const { return loop_depth_ > 0; }

    // 元组相关成员
    std::vector<TokenType> tuple_element_types_;  // 当前元组的元素类型

private:
    // 元组相关辅助方法
    bool is_tuple_type(TokenType type) const {
        return type == TokenType::KW_TUPLE;
    }

    bool is_array_type(TokenType type) const {
        return type == TokenType::KW_ARRAY;
    }

    /**
     * @brief 检查两个元组类型是否兼容
     * @param expected 期望的元组类型
     * @param actual 实际的元组类型
     * @return 如果类型兼容返回 true
     */
    bool is_tuple_compatible(const std::vector<TokenType>& expected,
                           const std::vector<TokenType>& actual) const {
        if (expected.size() != actual.size()) return false;
        for (size_t i = 0; i < expected.size(); ++i) {
            if (!is_compatible_type(expected[i], actual[i])) {
                return false;
            }
        }
        return true;
    }

    // -----------------------------------------------------------------------------
    // 成员变量
    // -----------------------------------------------------------------------------
    SymbolTable symbols_;                      ///< 符号表
    std::vector<SemanticError> errors_;        ///< 错误列表
    bool in_panic_mode_ = false;               ///< 是否在错误恢复模式
    TokenType current_type_ = TokenType::INVALID; ///< 当前表达式的类型
    Symbol* current_function_ = nullptr;        ///< 当前正在分析的函数
    bool has_return_ = false;                  ///< 当前路径是否有返回值
    int loop_depth_ = 0;                       ///< 循环嵌套深度
    std::vector<Token> tokens_;                ///< token 序列
    size_t current_token_index_ = 0;           ///< 当前 token 索引
    TokenType array_element_type_ = TokenType::INVALID; ///< 当前数组的元素类型

    // -----------------------------------------------------------------------------
    // 错误处理相关方法
    // -----------------------------------------------------------------------------
    template<typename Func>
    void with_error_handling(Func&& func);

    // -----------------------------------------------------------------------------
    // 类型转换相关方法
    // -----------------------------------------------------------------------------
    /**
     * @brief 检查类型是否支持比较运算
     * @param left 左操作数类型
     * @param right 右操作数类型
     * @return 如果类型可以比较返回 true
     */
    bool is_comparable_type(TokenType left, TokenType right) const;

    // 类型检查辅助函数
    bool is_valid_type(TokenType type) const;
};

} // namespace collie

#endif // COLLIE_SEMANTIC_ANALYZER_H
