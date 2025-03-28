---
sidebar_position: 1
sidebar_label: 介绍
---

# 介绍

《Collie 语言参考手册》

## 语言设计概述

Collie 是一种 AOT（Ahead Of Time; 先编译后运行）的编程语言，设计灵感来源于 C# 和 Java，并借鉴了其他编程语言（如：JavaScript, Python）的特性。该语言旨在降低编程难度，同时保持高效的运算能力，并提供良好的类型系统。

### 核心设计理念
- **null 安全**：Collie 语言基础类型不允许为 null，针对可能为 null 的场景，null 也是一个对象，实现了 object 的所有方法。使用 object? 问号语法显式声明，避免像 Java 语言那样需要对变量进行大量 null 判断。
- **隐式类型转换**：对于可能丢失精度的情况，Collie 不支持隐式的数字类型转换，需显式指定；对于不会丢失精度的情况，支持隐式数字类型转换。
- **数组与列表统一设计**：Collie 不区分数组 (Array) 与列表 (List)，统一使用 List。
- **消除歧义**：对于可能引起歧义的括号，必须添加。不支持逗号表达式，以免引起不必要的起义。
- **字符串遍历安全**：使用 UTF-16 编码格式作为默认编码，`character` 代表一个字符（无论一个字符占几个字节）。对于 BMP 范围外的字符（例如字符 `'𐍈'`），无需特殊处理。
