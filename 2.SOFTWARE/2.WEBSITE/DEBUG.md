# 网站调试记录

> GeekEgret Studio 网站 (index.html) 运行于 Rockchip RV1106 + Nginx

---

## 1. Splash 欢迎页透明导致与主页叠加

**日期**: 2026-06-04

### 现象

首次访问网站时，欢迎页（Splash Screen）背景透明，导致主页内容透过欢迎页同时显示，两个界面叠加在一起。

### 根因

1. CSS 变量 `--theme` 在 `:root`、`@media (prefers-color-scheme: dark)`、`:root.dark` 三处均设置为 `transparent`
2. `#splash` 使用 `background: var(--theme)`，即 `background: transparent`
3. JS 中还硬编码了 `splash.style.background = 'transparent'`

### 修复

- `#splash` 的背景色改为固定值 `#0a0a12`，不再依赖 `--theme` 变量
- 删除 JS 中的 `splash.style.background = 'transparent'` 行

### 相关文件

- `index.html`: CSS 第 258 行，JS 第 389 行（已删除）

---

## 2. 从其他页面返回首页时出现 404

**日期**: 2026-06-04

### 现象

在文章页或产品页点击"首页"菜单或"返回首页"链接时，显示 404。

### 根因

综合分析有 3 个代码问题：

| # | 位置 | 问题 | 后果 |
|---|------|------|------|
| 1 | 菜单链接 | `href="index.html"` | SPA 路由器拼接成 `index.html.html` |
| 2 | 路由器 | `location.reload()` | pushState 后 URL 已变为 `/articles` 等，reload 请求错误路径 |
| 3 | `loadPage()` | `path + '.html'` | 对已含 `.html` 的路径会生成 `.html.html` |

### 修复

| # | 修改 |
|---|------|
| 1 | 所有内部链接统一使用 `/` (logo、菜单、页脚、top-link、404 兜底) |
| 2 | `location.reload()` 改为 `window.location.href = '/'`，强制跳转到根路径 |
| 3 | `loadPage()` 中增加 `\.html$/.test(path)` 判断，避免重复追加 `.html` |

### 相关文件

- `index.html`: SPA 路由脚本区域

---

## 3. 产品 / 文章详情页返回 403

**日期**: 2026-06-04

### 现象

点击首页「最新动态」的文章链接，或进入产品/文章列表后点击详情卡片，页面返回 403。

### 根因

`articles/` 和 `products/` 目录权限为 `drwx------` (700)，属主为 `root`。Nginx worker 进程以非 root 用户（如 `nobody`）运行，无法进入目录读取文件。顶层 `/usr/html/` 目录权限正常（755），所以 `index.html`、`articles.html` 等顶层文件可访问。

### 修复

```sh
chmod 755 /usr/html/articles /usr/html/products
chmod 644 /usr/html/articles/*.html /usr/html/products/*.html
```

### 预防

`scp` 拷贝文件时会保留源端目录权限。建议部署后在板卡上执行权限修复，或在打包脚本中统一设置。

### 相关文件

- 板卡端: `/usr/html/articles/`, `/usr/html/products/`

---

---

## 4. 浏览器缓存导致删除文章后仍显示

**日期**: 2026-06-06

### 现象

板卡上删除了 5 篇 HTML 文章文件后，电脑浏览器仍显示已删除的文章。

### 根因

1. 无 Cache-Control 响应头，浏览器默认缓存 HTML 和 JS 资源
2. `fetch()` 调用未禁用缓存，SPA 动态加载页面时使用浏览器缓存
3. 无文件名哈希 / 版本号等缓存破坏机制

### 修复

- `index.html` 添加 3 个 meta 标签：`Cache-Control: no-cache, no-store, must-revalidate`、`Pragma: no-cache`、`Expires: 0`
- 所有 `fetch()` 调用添加 `{ cache: 'no-store' }` 参数
- 同步删除本地多余的 5 个 HTML 文章文件：`edge-inference.html`, `geekegret-studio.html`, `rv1106-camera.html`, `sensor-network.html`, `smart-home.html`
- 更新 `index.html` 首页和 `articles.html` 文章列表移除已删除文章的硬编码引用

### 相关文件

- `index.html`: `<head>` meta 标签 + 所有 `fetch()` 调用
- `articles.html`: 文章列表
- 本地 & 板卡 `articles/` 目录

---

## 5. 文章/产品列表改为全动态

**日期**: 2026-06-06

### 现象

每新增或删除一篇文章/产品，需要手动修改 `index.html` 首页列表、`articles.html` 文章列表、`products.html` 产品列表等多处引用。

### 根因

文章和产品列表全部硬编码在 HTML 中，缺乏动态发现机制。

### 修复

**板卡端 (Nginx)**:
- 新增 `/articles-json/` 和 `/products-json/` 两个 location，启用 `autoindex on; autoindex_format json;`，返回对应目录的文件 JSON 清单
- `index.html` meta 标签：`Cache-Control: no-cache, no-store, must-revalidate`、`Pragma: no-cache`、`Expires: 0`
- 所有 `fetch()` 调用添加 `{ cache: 'no-store' }` 参数

**客户端 (JS)**:
- 新增 `fetchArticleList()` / `fetchProductList()`：请求 autoindex JSON，过滤 `.md/.html` 文件，按 mtime 降序排列
- 新增 `getItemMeta(file, basePath)`：对每个文件发起 fetch，自动提取标题（`<h1>` 或 `# heading`）、标签、日期，按 `文件名@mtime` 缓存到 `sessionStorage`
- 新增 `extractTagsFromMd()` / `extractTagsFromHtml()`：Markdown 从 `tags:` 行提取，HTML 从 `<span class="tag">` 或 `<div class="detail-meta">标签：` 提取
- 新增 `renderHomeArticleList()` / `renderArticlesPage()` / `renderProductsPage()`：动态生成卡片 HTML
- 首页加载时自动请求最新 6 篇文章；`/articles`、`/products` 路由展示全部

**使用方式**:
- 往 `articles/` 或 `products/` 扔 `.md` / `.html` 文件，scp 到板卡即可，无需修改任何 HTML 引用
- Markdown 文件首行可加 `tags: 标签1, 标签2` 声明标签
- HTML 文件标题从 `<h1>` 自动提取，标签从 tag class 或 detail-meta 自动提取

### 相关文件

- 板卡: `/etc/nginx/nginx.conf`
- `index.html`: 动态文章系统 JS (~200 行)
- `articles/welcome.md`: 添加 `tags:` 元数据示例

---

## 6. 非首页路由刷新报 403

**日期**: 2026-06-06

### 现象

在 `/articles`、`/products`、`/about`、`/contact` 等页面刷新浏览器时，Nginx 返回 403。

### 根因

Nginx 无 `try_files` 配置。`/articles` 请求匹配到 `/usr/html/articles/` 目录，目录存在但无 `index.html` 且 `autoindex` 未开启，返回 403。SPA 仅在点击链接时通过 `pushState + fetch` 工作，刷新时由 Nginx 直接处理路由。

### 修复

1. Nginx `location /` 添加 `try_files $uri /index.html;`（不含 `$uri/` 避免匹配目录后停止回落）
2. JS 新增 `initRoute()` 立即执行函数：页面加载时读取 `location.pathname`，若非首页则调用 `loadPage()` 渲染对应页面内容，并正确高亮导航栏

### 验证

```bash
curl -s -o /dev/null -w "%{http_code}" http://192.168.1.9/articles  # 200
curl -s -o /dev/null -w "%{http_code}" http://192.168.1.9/products  # 200
curl -s -o /dev/null -w "%{http_code}" http://192.168.1.9/about     # 200
```

### 相关文件

- 板卡: `/etc/nginx/nginx.conf`
- `index.html`: `initRoute()` 函数

---

## 7. 首页标语修改 & 产品文件部署

**日期**: 2026-06-06

### 修改

- 首页个人简介标语从 "请用绝对的理智和清醒的头脑，探索嵌入式与 AIoT 的无限可能。" 改为 "拥抱开源，回馈开源。"
- 产品目录 4 个 HTML 文件首次部署到板卡 `/usr/html/products/`，并修复权限 `chmod 644`

### 相关文件

- `index.html`: 首页 `<span>` 标语
- 板卡: `/usr/html/products/*.html`

---

## 8. 提取所有可调参数到 param.json

**日期**: 2026-06-06

### 背景

网站所有文案、链接、配置项硬编码在 `index.html` 中。每次修改标题、社交链接、页面文案等，都需要编辑 HTML 文件并重新部署。没有集中的配置入口。

### 修改

**新增 `param.json`**：汇总所有可调参数，共 6 大类 ~40 个配置项：

| 分类 | 包含参数 |
|------|----------|
| `meta` | title, keywords, description, author, themeColor |
| `nav` | 导航菜单项数组（label + href） |
| `profile` | avatar, logo, name, tagline |
| `social` | github, bilibili, taobao, email, rss |
| `pages` | 首页/文章页/产品页的标题、副标题、空状态文案 |
| `footer` | poweredBy HTML |
| `particles` | enabled, blur, densityDivisor, maxStars |
| `splash` | enabled, duration, background |

**JS 加载器**：在 `index.html` 首个 `<script>` 中 fetch `param.json`，存入 `window.__P`。DOMContentLoaded 时更新：
- `<title>` + meta 标签
- Logo 文字
- 导航栏（完全由 JSON 动态生成）
- 个人信息区（头像、名称、标语）
- 社交图标链接
- 页脚 HTML
- 首页文章区标题

**其他 JS 适配**：
- 粒子系统：`enabled` 可开关星空背景，`blur`/`densityDivisor`/`maxStars` 可调
- Splash 动画：`enabled`/`duration`/`background` 可配置
- `renderArticlesPage`/`renderProductsPage`：页面标题、副标题、空状态文案均从 `__P` 读取
- 首页初始化：`maxArticles` 从 `__P.pages.home.maxArticles` 读取

**容错**：param.json 不存在或格式错误时，`window.__P` 为 `null`，所有代码回退到 HTML 默认值，网站正常运行。

### 使用方式

```bash
# 1. 修改本地 param.json
# 2. 部署到板卡
sshpass -p 'geekegret' scp param.json root@192.168.1.9:/usr/html/
# 3. 修复权限
ssh root@192.168.1.9 'chmod 644 /usr/html/param.json'
# 4. 刷新浏览器
```

### 相关文件

- `param.json`: 新建的全局配置文件
- `index.html`: 新增 ~80 行 param loader + 适配代码
- `USER.md`: 更新第 2 节自定义参数文档 + 全文档同步

---

## 9. 星空背景改为宇宙效果 + 高斯模糊

**日期**: 2026-06-06

### 现象

原有的粒子连线网络效果不像宇宙星空，且清晰度过高影响前景阅读。

### 修复

- 移除粒子间连线 + 鼠标交互，改为三层星空（暗星/亮星/带光晕恒星）
- 每颗星独立 sin 波闪烁，亮星带十字星芒 + 径向渐变光晕
- Canvas 叠加深蓝色星云 `radial-gradient` 底色
- 添加 `filter: blur(1.5px)` 高斯模糊柔化背景
- 粒子系统全面响应式——放大缩小时星星密度和坐标同步调整
- JS 跳过 Markdown `tags:` 元数据行，不再渲染到正文
- 文章/产品列表页新增 tag 筛选栏（从所有条目提取唯一标签，点击过滤）
- 所有参数接入 `param.json` 的 `particles` 配置

### 相关文件

- `index.html`: 粒子系统（~120 行） + param loader

### 测试文件是否可访问

```bash
# 在板卡上直接 curl 测试
curl -s -o /dev/null -w "%{http_code}" http://127.0.0.1/articles/rv1106-camera.html
# 预期: 200
# 403: 权限问题
# 404: 文件不存在或路径错误
```

### 检查目录权限

```bash
ssh root@192.168.1.9 'ls -laR /usr/html/'
```

### 检查 Nginx 错误日志

```bash
ssh root@192.168.1.9 'cat /var/log/nginx/error.log'
```

---

## 10. Markdown 表格解析修复

**日期**: 2026-06-06

### 现象

Markdown 文件中的多列表格（如 `| H1 | H2 | H3 |`）无法正确渲染，表格分隔行被当作普通文本显示。此外，`.md` 文件在浏览器中被当作二进制文件下载而非直接显示，中文 JSON 文件出现乱码。

### 根因

1. 多列分隔符正则 `/^\|[\s\-:]+\|$/` 只匹配单列分隔符 `|---|`，多列分隔符 `|---|---|` 中包含的 `|` 不在字符类中，导致匹配失败
2. `splitCells()` 函数直接按 `|` 字符拆分单元格，未保护行内代码 `` `...` `` 和转义管道 `\|` 中的 `|`
3. Nginx 未配置 `.md` 文件的 MIME 类型，默认返回 `application/octet-stream` 触发浏览器下载
4. 未设置 `charset utf-8`，导致 JSON 和 Markdown 中文乱码

### 修复

- 正则改为 `/^\|[\s\-:\|]+\|$/`，在字符类中加入 `\|`
- `splitCells()` 增加占位符保护机制：先用临时标记替换行内代码和转义管道，`split('|')` 拆分单元格后再还原
- Nginx `http` 块添加 `charset utf-8;` 全局编码声明
- Nginx `http` 块添加 `text/markdown md;` MIME 类型

### 相关文件

- `index.html`: Markdown 解析器正则 + `splitCells()` 函数
- 板卡: `/etc/nginx/nginx.conf`

---

## 11. param.json 注释剥离导致 JSON 解析失败

**日期**: 2026-06-06

### 现象

网站加载后 `window.__P` 为 `null`，所有 param.json 配置失效，回退到 HTML 默认值。浏览器控制台显示 JSON 解析错误。

### 根因

JS loader 使用 `replace(/\/\/.*$/gm, '')` 试图剥离 JSON 中的注释，但该正则错误匹配了 URL 中的 `://` 前缀（如 `https://github.com/...`），将 URL 后半部分截断，导致 JSON 格式损坏、`r.json()` 抛出异常。

### 修复

移除注释剥离逻辑。`param.json` 本身就是标准 JSON（无注释），直接使用 `r.json()` 解析即可。

### 相关文件

- `index.html`: param.json 加载脚本（删除 `replace(/\/\/.*$/gm, '')` 行）

---

## 12. loadPage 函数重复代码 + 括号失衡

**日期**: 2026-06-06

### 现象

所有非首页路由（`/articles`、`/products`、`/about`、`/contact`）均显示首页内容，导航完全失效。浏览器控制台报 JS 语法错误。

### 根因

重写 `loadPage` 函数时，旧代码尾部（约 65 行）未完全删除，留下了重复的事件处理器定义和一个多余的 `}` 大括号。多余括号导致整个脚本块括号失衡，JS 引擎无法解析该 `<script>` 块，后续 SPA 路由器未定义，所有 `fetch()` 调用失败、路由逻辑被跳过。

### 修复

- 删除残留的重复代码块
- 验证所有 3 个 `<script>` 块括号平衡：17/17、29/29、155/155

### 相关文件

- `index.html`: 第 2、3 个 `<script>` 块

---

## 13. .md 内容页刷新丢失 SPA 布局

**日期**: 2026-06-06

### 现象

在 `/articles/xxx` 页面刷新浏览器时，显示 `index.html` 的完整 SPA 首页布局而非文章内容。直接访问 `.md` 文件 URL 返回 200 但内容为首页 HTML。

### 根因

1. Nginx `try_files $uri /index.html;` 对所有未知路由返回 `index.html`（状态码 200）。当 `loadPage()` 请求一个不存在的 `xxx.html` 时，收到的是 `index.html` 的 HTML 内容（以 `<!DOCTYPE` 或 `<html` 开头），而非 404，导致 JS 误以为加载成功
2. 文章详情页 URL 中包含 `.md` 扩展名（如 `/articles/welcome.md`），不够简洁

### 修复

- 新增 `isIndexHtml()` 检查：如果 fetch 响应文本以 `<!DOCTYPE` 或 `<html` 开头，视为 404，回退尝试 `.md` 文件
- `.md` URL 在路由前通过 `replaceState` 去除扩展名，使文章链接显示为 `/articles/welcome`
- 新增 `META_CACHE_VER = 'v2'` 缓存前缀，使旧版缓存的 meta URL 失效

### 相关文件

- `index.html`: `loadPage()` 函数 + `META_CACHE_VER`

---

## 14. 星空背景粒子优化 + Tag 筛选 + 其它

**日期**: 2026-06-06

### 修改

- **粒子 Canvas 响应式**：窗口 resize 时按比例缩放已有粒子坐标，并自动增减粒子数维持密度
- **Tag 筛选栏**：`/articles` 和 `/products` 页面新增标签过滤条，从所有条目中提取唯一标签，点击筛选，默认"全部"。新增 `.tag-item.active` CSS 高亮样式
- **移除 RSS 图标**：从社交图标区、`param.json` social 对象和 JS keys 数组中移除 RSS 链接
- **新建 `edit_param.py`**：交互式 Python 配置编辑器，逐项展示参数用途和当前值，支持类型校验，修改后自动备份 `.bak` 文件

### 相关文件

- `index.html`: 粒子系统 + tag 筛选 UI + CSS + 社交图标逻辑

---

## 15. 关于页/联系页内容硬编码 + Email 解析 + 导航栏高亮

**日期**: 2026-06-06

### 现象

1. 联系页的 GitHub、Email 链接是硬编码的旧值，与 `param.json` 的 `social` 不同步
2. 点击首页邮箱图标跳转到 `http://192.168.1.9/18750211657@163.com`（缺少 `mailto:` 前缀）
3. 非首页刷新后导航栏始终高亮"首页"

### 根因

1. **联系页/关于页**：内容从 `contactPage.content` / `aboutPage.content` 静态 HTML 渲染，与 `social` 字段独立
2. **Email**：`param.json` 的 `email` 值缺少 `mailto:` 前缀，浏览器按相对路径解析
3. **param 加载时序**：`fetch(param.json)` 异步，`initRoute()` 同步执行时 `window.__P` 为 `null`，导致 `loadPage` 无法读取参数
4. **导航高亮**：param loader 生成导航 HTML 时硬编码 `class="active"` 在首页链接上，覆盖了 `initRoute` 的正确高亮

### 修复

1. 联系页/关于页改为从 `social` 动态生成链接：
   - `social.github` → 自动提取用户名显示 `@Geek-Egret`
   - `social.email` → 自动去掉 `mailto:` 前缀显示，链接保留完整 `mailto:`
   - `contactPage.content` / `aboutPage.content` 为空时走动态生成，填入自定义 HTML 时优先使用
2. `param.json` 的 `email` 补上 `mailto:` 前缀
3. 新增 `window.__P_ready` Promise：param 加载完成（成功或失败）后 resolve，`initRoute` 等待 Promise 再调用 `loadPage`
4. param loader 生成导航后根据当前 URL 重新设置高亮；`initRoute` 在 `__P_ready` resolve 后再次确认高亮

### 相关文件

- `index.html`: 联系页/关于页处理器 + `__P_ready` + 导航高亮逻辑
- `param.json`: `social.email` 修复 `mailto:` 前缀，`contactPage.content` / `aboutPage.content` 清空走动态生成
- `param.json`: 移除 `social.rss`
- `edit_param.py`: 新建（164 行）
