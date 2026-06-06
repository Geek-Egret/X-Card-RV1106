# 网站结构与维护手册

> GeekEgret Studio 网站运行于 Rockchip RV1106 + Nginx
> 板卡 IP: 192.168.1.9 | SSH: root/geekegret

---

## 1. 目录结构

```
/usr/html/                          # Nginx root (板卡端)
├── index.html                      # 主页 (SPA, 包含所有 CSS/JS)
├── param.json                      # 全局可调参数（★ 自定义入口）
├── about.html                      # 关于页面片段
├── articles.html                   # 文章列表页片段（已废弃，动态生成）
├── products.html                   # 产品列表页片段（已废弃，动态生成）
├── contact.html                    # 联系页面片段
├── 50x.html                        # 错误页面
├── assets/
│   ├── logo.png                    # 网站 Logo
│   └── avatar.png                  # 头像
├── edit_param.py                    # param.json 交互式配置编辑器
├── articles/                       # 文章详情（自动发现，无需手动引用）
│   └── welcome.md                  # Markdown 文章示例
└── products/                       # 产品详情（自动发现，无需手动引用）
    ├── rv1106-devkit.html
    ├── ai-cam-module.html
    ├── edge-gateway.html
    └── sensor-kit.html
```

本地源码位置: `GEEK-EGRET-CARD-RV1106/2.SOFTWARE/2.WEBSITE/`

---

## 2. 自定义网站参数（★ 新增）

网站所有可调参数集中在 **`param.json`** 文件中，修改后 scp 到板卡即可实时生效，无需修改 HTML/JS 代码。

### 2.1 参数速查表

| 分类 | 参数 | 默认值 | 说明 |
|------|------|--------|------|
| **meta** | `title` | `Leeeezy's Studio` | 浏览器标题栏 & Logo 文字 |
| | `keywords` | `geekegret,RV1106,...` | SEO 关键字 |
| | `description` | `拥抱开源，回馈开源。` | 页面描述 |
| | `author` | `geekegret` | 网站作者 |
| | `themeColor` | `#1a1a2e` | 浏览器主题色 |
| **nav** | `[]` | 5 个菜单项 | 导航栏，可增减改 |
| **profile** | `avatar` | `assets/avatar.png` | 头像路径 |
| | `name` | `GeekEgret Studio` | 工作室名称 |
| | `tagline` | `拥抱开源，回馈开源。` | 首页标语 |
| **social** | `github` | `https://github.com/...` | GitHub 链接 |
| | `bilibili` | `https://space.bilibili...` | B站链接 |
| | `taobao` | `https://shop.m.taobao...` | 淘宝链接 |
| | `email` | `geekegret@example.com` | 邮箱（需含 `mailto:` 前缀） |
| **pages.home** | `sectionTitle` | `📝 最新动态` | 首页文章区标题 |
| | `maxArticles` | `6` | 首页最多显示文章数 |
| **pages.articles** | `title` | `📝 文章` | 文章页标题 |
| | `subtitle` | 工作室技术分享... | 文章页副标题 |
| | `emptyMsg` | `暂无文章` | 无文章提示 |
| | `noMatchMsg` | `无匹配文章` | tag 筛选无结果 |
| **pages.products** | `title` | `🛒 产品` | 产品页标题 |
| | `subtitle` | 工作室推出... | 产品页副标题 |
| | `emptyMsg` | `暂无产品` | 无产品提示 |
| | `noMatchMsg` | `无匹配产品` | tag 筛选无结果 |
| **footer** | `poweredBy` | `Powered by Nginx...` | 页脚 HTML |
| **particles** | `enabled` | `true` | 星空背景开关 |
| | `blur` | `1.5` | 星空模糊度(px) |
| | `densityDivisor` | `2200` | 星星密度 (面积除以该值) |
| | `maxStars` | `500` | 最大星星数量 |
| **splash** | `enabled` | `true` | 开机动画开关 |
| | `duration` | `1500` | 动画时长(ms) |
| | `background` | `#0a0a12` | 动画背景色 |

### 2.2 自定义示例

**改标题 & 标语：**
```json
"meta": { "title": "我的工作室" },
"profile": { "tagline": "探索技术的无限可能。" }
```

**禁用星空背景：**
```json
"particles": { "enabled": false }
```

**改首页显示文章数：**
```json
"pages": { "home": { "maxArticles": 3 } }
```

**添加导航菜单项：**
```json
"nav": [
  { "label": "首页", "href": "/" },
  { "label": "博客", "href": "/articles" },
  { "label": "友链", "href": "/links" }
]
```

### 2.3 交互式编辑器（edit_param.py）

提供了 Python 交互式脚本 `edit_param.py`，逐项展示每个参数的用途和当前值，可选择修改或跳过。修改后自动生成 `.bak` 备份，支持类型校验。

```bash
cd 2.SOFTWARE/2.WEBSITE
python3 edit_param.py
```

运行示例：
```
[meta.title]
  用途: 浏览器标题 & Logo 文字
  当前: "Leeeezy's Studio"
  新值: [输入新值回车 / 直接回车跳过 / q 退出保存]
```

> 注：导航栏 (`nav`) 为数组格式，建议直接编辑 JSON 文件或通过 param.json 手动修改。

### 2.4 生效方式

1. 修改本地 `param.json`
2. scp 到板卡：`sshpass -p 'geekegret' scp param.json root@192.168.1.9:/usr/html/`
3. 修复权限：`ssh root@192.168.1.9 'chmod 644 /usr/html/param.json'`
4. 浏览器强制刷新（Ctrl+Shift+R）即可看到变化

> 如果 param.json 不存在或格式错误，网站会使用 index.html 中的默认值正常运行。

---

## 3. 技术架构

### SPA 单页应用

网站采用 SPA (Single Page Application) 架构：

- `index.html` 是唯一的完整页面（含 header、footer、样式、脚本）
- `param.json` 是全局配置文件，页面加载时通过 JS 动态读取并应用
- 文章/产品列表通过 Nginx autoindex JSON + JS 动态发现，无需手动维护引用
- 导航链接通过 JS 拦截，`fetch()` 加载片段并替换 `<main>` 区域
- URL 通过 `history.pushState()` 同步更新，支持浏览器前进/后退
- Nginx `try_files $uri /index.html;` 保证刷新任何页面都返回 200

### 导航路由表

| 菜单 | href | 行为 |
|------|------|------|
| 首页 | `/` | 完整页面刷新 |
| 产品 | `/products` | JS fetch `/products-json/` 动态渲染 |
| 文章 | `/articles` | JS fetch `/articles-json/` 动态渲染 |
| 关于 | `/about` | JS fetch `about.html` 片段 |
| 联系 | `/contact` | JS fetch `contact.html` 片段 |

详情页路由会尝试加载 `articles/xxx.html`，若不存在则尝试 `articles/xxx.md`。

---

## 4. 部署到板卡

### 方式一：部署全部文件

```bash
sshpass -p 'geekegret' scp -r \
  2.SOFTWARE/2.WEBSITE/* \
  root@192.168.1.9:/usr/html/
```

### 方式二：部署单个文件

```bash
sshpass -p 'geekegret' scp \
  2.SOFTWARE/2.WEBSITE/param.json \
  root@192.168.1.9:/usr/html/
```

### 部署后修复权限

```bash
ssh root@192.168.1.9 'chmod 755 /usr/html/articles /usr/html/products'
ssh root@192.168.1.9 'chmod 644 /usr/html/param.json'
ssh root@192.168.1.9 'chmod 644 /usr/html/articles/* /usr/html/products/*'
```

---

## 5. 添加新文章 / 产品（★ 自动发现）

文章和产品列表已实现全动态——只需将文件放入对应目录即可，无需修改任何 HTML。

### 文章

1. 在 `articles/` 目录创建 `.md` 或 `.html` 文件
2. Markdown 文件可在首行加 `tags: 标签1, 标签2` 声明标签
3. HTML 文件标题从 `<h1>` 自动提取，标签从 `<span class="...tag...">` 或 `<div class="detail-meta">标签：</div>` 自动提取
4. 部署到板卡后，首页和文章列表页自动出现

### 产品

1. 在 `products/` 目录创建 `.html` 文件
2. 格式参照已有产品文件（`<div class="project-detail"><h1>标题</h1><div class="detail-meta">标签：xx / yy</div>...</div>`）
3. 部署后自动出现在产品列表页

---

## 6. Nginx 配置

配置文件位置: `/etc/nginx/nginx.conf`（板卡端）

关键配置：
```nginx
charset utf-8;
types { text/markdown md; }

server {
    listen 80;
    location / {
        root html;
        index index.html;
        try_files $uri /index.html;
    }
    location /articles-json {
        alias /usr/html/articles;
        autoindex on;
        autoindex_format json;
    }
    location /products-json {
        alias /usr/html/products;
        autoindex on;
        autoindex_format json;
    }
}
```

> `charset utf-8;` 确保 JSON 和 Markdown 中文正确编码；`text/markdown md;` 使 `.md` 文件在浏览器中直接展示而非下载。

---

## 7. Markdown 支持说明

### 原理

内置轻量 Markdown 解析器（~2KB），支持常见语法。`loadPage()` 加载文件逻辑：

1. 路径以 `.md` 结尾 → fetch `.md` 并解析渲染
2. 路径以 `.html` 结尾 → 直接加载 HTML 片段
3. 无扩展名 → 先尝试 `.html`，404 则尝试 `.md`

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
| `\| 列1 \| 列2 \| 列3 \|` | 表格（支持多列、带对齐分隔符） |
| `---` | 分割线 |

### 元数据行

Markdown 文件首行支持 `tags:` 元数据，示例：
```markdown
tags: RV1106, 教程, 嵌入式
# 文章标题
```
此行会被文章列表页作为标签提取，且不会显示在正文中。

---

## 8. 常见问题

### Q: 修改 param.json 后没生效？
1. 确认文件权限 644：`chmod 644 /usr/html/param.json`
2. 浏览器强制刷新：Ctrl+Shift+R
3. 验证可访问：`curl http://192.168.1.9/param.json`

### Q: 文章 404？
检查文件是否存在，确认目录权限 755。

### Q: 文章 403？
目录权限不足：`chmod 755 /usr/html/articles /usr/html/products`；文件权限：`chmod 644`

### Q: 刷新非首页 403？
Nginx 已配置 `try_files $uri /index.html;`，确认配置已生效。

### Q: 表格渲染不正确？
多列表格分隔符已修复（v2026-06-06+），确认使用的是最新版 `index.html`。若 `.md` 文件被浏览器下载而非显示，检查 Nginx 是否配置了 `text/markdown md;` MIME 类型。

### Q: param.json 修改后网站回退到默认值？
1. 确认 JSON 格式正确（无注释、引号配对）：`python3 -c "import json; json.load(open('param.json'))"`
2. 不要使用 `//` 注释，param.json 必须为纯 JSON
3. 部署后修复权限：`chmod 644 /usr/html/param.json`

### Q: 非首页路由显示首页内容？
检查浏览器控制台是否有 JS 语法错误。确认 `index.html` 中 `<script>` 块括号平衡。

### Q: .md 详情页刷新后显示首页？
确认 Nginx `try_files` 配置正确，且 `index.html` 中 `loadPage()` 含 `isIndexHtml()` 检查。

### Q: 修改 social 后联系页/关于页链接没变？
联系页和关于页从 `social` 字段动态生成链接。如果 `contactPage.content` 或 `aboutPage.content` 非空，则优先使用自定义 HTML（此时 `social` 变更不会自动反映）。清空这两个 content 字段即可切回动态生成。

### Q: 点击邮箱图标跳转错误？
`social.email` 必须包含 `mailto:` 前缀，如 `"mailto:geekegret@example.com"`，否则浏览器会将其当作相对路径。

### Q: 刷新非首页后导航栏高亮错误？
导航栏由 `param.json` 异步加载。页面加载时会先显示默认高亮，param 加载完成后会自动修正为当前页面。

### Q: 如何本地预览？
```bash
cd 2.SOFTWARE/2.WEBSITE
python3 -m http.server 8080
```

> 本地预览与板卡行为有差异：本地无 Nginx autoindex JSON 目录，文章/产品列表无法自动发现。建议在板卡上测试完整功能。
