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

## 排查技巧

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
