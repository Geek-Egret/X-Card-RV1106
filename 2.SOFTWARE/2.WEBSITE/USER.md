# 网站结构与维护手册

> GeekEgret Studio 网站运行于 Rockchip RV1106 + Nginx
> 板卡 IP: 192.168.1.9 | SSH: root/geekegret

---

## 1. 目录结构

```
/usr/html/                          # Nginx root (板卡端)
├── index.html                      # 主页 (SPA, 包含所有 CSS/JS)
├── about.html                      # 关于页面
├── articles.html                   # 文章列表页
├── products.html                   # 产品列表页
├── contact.html                    # 联系页面
├── 50x.html                        # 错误页面
├── assets/
│   ├── logo.png                    # 网站 Logo
│   └── avatar.png                  # 头像
├── articles/                       # 文章详情
│   ├── rv1106-camera.html
│   ├── edge-inference.html
│   ├── geekegret-studio.html
│   ├── smart-home.html
│   ├── sensor-network.html
│   └── welcome.md                  # Markdown 文章示例
└── products/                       # 产品详情
    ├── rv1106-devkit.html
    ├── ai-cam-module.html
    ├── edge-gateway.html
    └── sensor-kit.html
```

本地源码位置: `GEEK-EGRET-CARD-RV1106/2.SOFTWARE/2.WEBSITE/`

---

## 2. 技术架构

### SPA 单页应用

网站采用 SPA (Single Page Application) 架构：

- `index.html` 是唯一的完整页面（含 header、footer、样式、脚本）
- 其他页面（about、articles、products、contact 及详情页）只包含 `<div class="page-content">...</div>` 片段
- 点击导航链接时，JS 拦截点击，通过 `fetch()` 加载片段并替换 `<main>` 区域
- URL 通过 `history.pushState()` 同步更新，支持浏览器前进/后退

### 导航路由表

| 菜单 | href | 加载的文件 | 说明 |
|------|------|-----------|------|
| 首页 | `/` | 完整页面刷新 | 回到主页 |
| 产品 | `/products` | `products.html` | 产品列表 |
| 文章 | `/articles` | `articles.html` | 文章列表 |
| 关于 | `/about` | `about.html` | 关于页 |
| 联系 | `/contact` | `contact.html` | 联系页 |

详情页路由（`href="/articles/xxx"`）会尝试加载 `articles/xxx.html`，若不存在则尝试 `articles/xxx.md`。

---

## 3. 部署到板卡

### 方式一：部署全部文件

```bash
sshpass -p 'geekegret' scp -r \
  2.SOFTWARE/2.WEBSITE/* \
  root@192.168.1.9:/usr/html/
```

### 方式二：部署单个文件

```bash
sshpass -p 'geekegret' scp \
  2.SOFTWARE/2.WEBSITE/index.html \
  root@192.168.1.9:/usr/html/
```

### 部署后修复权限

```bash
ssh root@192.168.1.9 'chmod 755 /usr/html/articles /usr/html/products'
ssh root@192.168.1.9 'chmod 644 /usr/html/articles/*.html /usr/html/products/*.html'
```

---

## 4. 添加新文章

### HTML 文章

1. 在 `articles/` 目录创建 `my-article.html`
2. 文件内容只包含 `<div class="page-content">...</div>` 片段
3. 在 `index.html` 的最新动态区域添加链接：
   ```html
   <a href="/articles/my-article" class="entry-item">...</a>
   ```
4. 在 `articles.html` 的文章列表中添加卡片：
   ```html
   <a href="/articles/my-article" class="project-card">...</a>
   ```

### Markdown 文章（推荐）

1. 在 `articles/` 目录创建 `my-article.md`
2. 使用 Markdown 语法编写内容（支持标题、列表、代码块、表格等）
3. 在首页添加链接（可带 `.md` 后缀或不带）：
   ```html
   <a href="/articles/my-article.md" class="entry-item">...</a>
   ```
4. 部署到板卡

### 添加产品页

步骤同上，在 `products/` 目录操作，在 `products.html` 中更新列表。

---

## 5. 自定义样式

所有 CSS 样式集中在 `index.html` 的 `<style>` 标签中：

- CSS 变量（主题色）定义在 `:root` 选择器中
- 修改颜色变量可统一改变整体风格
- 修改 `--theme` 变量会影响 body/html 背景色，但不会影响 splash 屏（splash 使用固定色 `#0a0a12`）

### 关键 CSS 变量

```css
:root {
  --theme: transparent;           /* body 背景 */
  --entry: rgba(255,255,255,0.07); /* 卡片背景 */
  --primary: rgba(255,255,255,0.92); /* 主文字 */
  --secondary: rgba(255,255,255,0.6); /* 次要文字 */
  --border: rgba(255,255,255,0.22); /* 边框 */
  --code-bg: rgba(255,255,255,0.08); /* 行内代码背景 */
  --code-block-bg: rgba(255,255,255,0.12); /* 代码块背景 */
}
```

---

## 6. Nginx 配置

配置文件位置: `/etc/nginx/nginx.conf`（板卡端）

关键配置：
```
server {
    listen       80;
    server_name  localhost;
    root         html;        # 即 /usr/html/
    index        index.html index.htm;
}
```

当前没有配置 `try_files` 和 URL rewrite。SPA 的路由由前端 JS 的 pushState + fetch 实现。

---

## 7. Markdown 支持说明

### 原理

网站内置了一个轻量级 Markdown 解析器（在 `index.html` 的 JS 中），约 2KB，无需外部依赖。`loadPage()` 函数加载文件时的逻辑：

1. 若路径以 `.md` 结尾 → 直接 fetch `.md` 文件，解析后渲染
2. 若路径以 `.html` 结尾 → 直接加载 HTML 片段
3. 若路径无扩展名 → 先尝试 `.html`，404 则尝试 `.md`

### 支持的语法

| 语法 | 渲染结果 |
|------|----------|
| `# H1 ~ #### H4` | 标题 1-4 |
| `**粗体**` `*斜体*` `***粗斜体***` | 文字样式 |
| `~~删除线~~` | ~~删除线~~ |
| `[文字](URL)` | 超链接 |
| `![替代](URL)` | 图片 |
| `` `code` `` | 行内代码 |
| ` ```语言\n代码块\n``` ` | 代码块 |
| `- 项目` `* 项目` | 无序列表 |
| `1. 项目` | 有序列表 |
| `> 引用` | 引用块 |
| `\| 列1 \| 列2 \|` | 表格 |
| `---` | 分割线 |

### 注意事项

- 代码块内容会自动 HTML 转义，`<script>` 等不会被浏览器执行（安全）
- 外链默认添加 `target="_blank"` 和 `rel="noopener"`
- 表格需要表头行 + 分隔行格式

---

## 8. 常见问题

### Q: 文章 404？

检查文件是否存在于对应目录，确认目录权限为 755（`ls -la /usr/html/articles/`）。

### Q: 文章 403？

目录权限不足，执行：`chmod 755 /usr/html/articles /usr/html/products`

### Q: SPA 路由不生效？

刷新页面后如果直接访问 `/articles` 等路径会 404，因为 nginx 没有配置 fallback。建议从首页开始导航，或配置 nginx `try_files`。

### Q: 如何本地预览？

直接用浏览器打开 `index.html` 即可预览静态样式。SPA 路由的 fetch 在本地 `file://` 协议下会因跨域限制失效，建议用 `python3 -m http.server` 在本地起一个 HTTP 服务。

```bash
cd 2.SOFTWARE/2.WEBSITE
python3 -m http.server 8080
# 浏览器访问 http://localhost:8080
```
