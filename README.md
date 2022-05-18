# HITSZ-maillab

指导网站：https://hitsz-cslab.gitee.io/comp-network/lab9/index.html

实现一个简单的邮件客户端，完成邮件发送与接收功能：

- 使用 STMP 协议与邮件服务器交互，将邮件发送给任意收件人。
- 使用 POP3 协议与服务器交互，查询收件箱中的邮件信息。



## Send your emails

1. 在 `send.c` 文件中更改 `host_name`、`user`、`pass`、`from` 为邮件服务器名称（如 `smtp.qq.com`）、你的邮箱（如 `username@qq.com`）、你的授权码（请自行在邮箱设置中获取）和发件人邮箱。

2. 编译程序：

   ```shell
   make
   ```

3. 在终端中执行以下命令：

   ```shell
   ./send RECIPIENT [-s SUBJECT] [-m MESSAGE] [-a ATTACHMENT]
   ```

   其中各参数为：

   - `RECIPIENT`：收件人邮箱

   - `SUBJECT`：邮件主题

   - `MESSAGE`：邮件正文 或 含有邮件正文的文件路径

     程序会首先检查 `MESSAGE` 是否是文件路径，如果是则读取文件内容作为正文，否则直接将其作为正文

   - `ATTACHMENT`：邮件附件，只支持一个附件



## Receive your emails

邮件收取功能相对固定，将按顺序执行以下操作：查看邮件总数和总大小，列出所有邮件和它们的大小，展示第一封邮件的内容。

1. 在 `recv.c` 文件中更改 `host_name`、`user`、`pass` 为邮件服务器名称（如 `pop.qq.com`）、你的邮箱（如 `username@qq.com`）、你的授权码（请自行在邮箱设置中获取）。

2. 编译程序：

   ```shell
   make
   ```

3. 在终端中执行以下命令：

   ```shell
   ./recv
   ```

