# Alpha 项目

## 简介
Alpha 项目是一个高性能的分布式系统框架，提供灵活的通道通信机制和RPC远程调用功能。

## 目录
- [快速开始](#快速开始)
- [更新日志](#更新日志)
- [贡献指南](#贡献指南)
- [联系方式](#联系方式)

## 快速开始 <a name="快速开始"></a>
按照以下步骤快速开始您的Alpha项目之旅：

1. 克隆项目到本地：
   ```
   git clone https://github.com/AlphaMinZ/Alpha.git
   ```
2. 进入项目目录
    ```
    cd Alpha
    ```
3. 创建编译所需的目录
    ```
    mkdir bin && mkdir build
    ```
4. 进入构建目录并运行CMake：
    ```
    cd build && cmake .. && make
    ```

更新日志 <a name="更新日志"></a>
- 2024-04-03: 引入的Chan结构特性如下：
阻塞 Chan：当Chan满时，发送操作将阻塞，直到Chan中的数据被其他协程读取并唤醒发送端。读操作同理。
非阻塞 Chan：在Chan满时尝试写入将直接返回写失败，而在Chan空时尝试读取将返回空指针，表明Chan为空。

- 2024-06-21：经过半个月的开发，添加了RPC远程程序调用功能。目前，显而易见的BUG已被修复，并且所有测试用例均已通过。项目分享地址 https://www.bilibili.com/video/BV1NF3ne9EoJ

贡献指南 <a name="贡献指南"></a>
我们非常欢迎并感谢您的贡献。请按照以下步骤进行：
1. Fork本项目
2. 创建您的分支以进行新特性或修复：git checkout -b feature/your-feature
3. 提交您的更改：git commit -am 'Add some feature'
4. 推送到您的分支：git push origin feature/your-feature
5. 提交一个Pull Request

联系方式 <a name="联系方式"></a>
如果您有任何问题或建议，请通过以下方式联系我们：
- 邮箱：mzwin2022@163.com
- QQ 群：255729313