#!/usr/bin/env python3
"""
GeekEgret Studio — param.json 交互式编辑器
逐项展示用途和当前值，可选择修改或跳过。
"""

import json
import sys
import os

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PARAM_PATH = os.path.join(SCRIPT_DIR, "param.json")

FIELDS = {
    "meta.title":            ("浏览器标题 & Logo 文字", str),
    "meta.keywords":         ("SEO 关键字", str),
    "meta.description":      ("页面描述", str),
    "meta.author":           ("网站作者", str),
    "meta.themeColor":       ("浏览器主题色 (#RRGGBB)", str),
    "meta.robots":           ("搜索引擎索引策略", str),

    "profile.avatar":        ("头像图片路径", str),
    "profile.logo":          ("Logo 图片路径", str),
    "profile.name":          ("工作室名称", str),
    "profile.tagline":       ("首页标语", str),

    "social.github":         ("GitHub 地址", str),
    "social.bilibili":       ("Bilibili 地址", str),
    "social.taobao":         ("淘宝店铺地址", str),
    "social.email":          ("联系邮箱", str),

    "pages.home.sectionTitle":   ("首页文章区标题", str),
    "pages.home.maxArticles":    ("首页最大文章数", int),
    "pages.articles.title":      ("文章页标题", str),
    "pages.articles.subtitle":   ("文章页副标题", str),
    "pages.articles.emptyMsg":   ("无文章时提示文字", str),
    "pages.articles.noMatchMsg": ("标签筛选无结果提示", str),
    "pages.products.title":      ("产品页标题", str),
    "pages.products.subtitle":   ("产品页副标题", str),
    "pages.products.emptyMsg":   ("无产品时提示文字", str),
    "pages.products.noMatchMsg": ("标签筛选无结果提示", str),

    "aboutPage.title":       ("关于页标题", str),
    "aboutPage.content":     ("关于页正文 (留空=使用默认)", str),
    "contactPage.title":     ("联系页标题", str),
    "contactPage.content":   ("联系页正文 (留空=使用默认)", str),

    "footer.poweredBy":      ("页脚 Powered by (支持HTML)", str),

    "particles.enabled":     ("启用星空背景 (true/false)", bool),
    "particles.blur":        ("星空模糊度 (px)", float),
    "particles.densityDivisor": ("星星密度除数 (越小越密)", int),
    "particles.maxStars":    ("最大星星数量", int),
    "particles.centerX":     ("银河中心 X 位置 (0~1)", float),
    "particles.centerY":     ("银河中心 Y 位置 (0~1)", float),
    "particles.rotationSpeed": ("银河旋转速度", float),
    "particles.arms":        ("旋臂数量", int),
    "particles.bgLayer1":    ("背景紫晕颜色 (rgba)", str),
    "particles.bgLayer2":    ("背景蓝晕颜色 (rgba)", str),
    "particles.bgLayer3":    ("背景暗紫翼颜色 (rgba)", str),
    "particles.coreColor":   ("宇宙底色 (#RRGGBB)", str),

    "splash.enabled":        ("启用开机动画 (true/false)", bool),
    "splash.duration":       ("动画持续时长 (ms)", int),
    "splash.background":     ("动画背景色 (暂未启用)", str),

    "nav":                   ("导航栏 (数组，格式复杂，建议手动编辑)", None),
}


def get_nested(data, path):
    keys = path.split(".")
    for k in keys:
        if isinstance(data, dict) and k in data:
            data = data[k]
        else:
            return None
    return data


def set_nested(data, path, value):
    keys = path.split(".")
    for k in keys[:-1]:
        data = data.setdefault(k, {})
    data[keys[-1]] = value


def format_val(v):
    if isinstance(v, str):
        return f'"{v}"'
    if isinstance(v, bool):
        return "true" if v else "false"
    return str(v)


def parse_val(raw, typ):
    raw = raw.strip()
    if typ is bool:
        low = raw.lower()
        if low in ("true", "1", "yes", "y"):
            return True
        if low in ("false", "0", "no", "n"):
            return False
        raise ValueError("请输入 true 或 false")
    if typ is int:
        return int(raw)
    if typ is float:
        return float(raw)
    return raw


def main():
    if not os.path.exists(PARAM_PATH):
        print(f"错误: 找不到 {PARAM_PATH}")
        sys.exit(1)

    with open(PARAM_PATH, "r", encoding="utf-8") as f:
        data = json.load(f)

    print("=" * 60)
    print("  GeekEgret Studio — param.json 配置编辑器")
    print("  输入新值按回车确认，直接回车跳过，输入 q 退出保存")
    print("=" * 60)

    changed = 0
    for path, (desc, typ) in FIELDS.items():
        if typ is None:
            continue

        current = get_nested(data, path)
        print(f"\n[{path}]")
        print(f"  用途: {desc}")
        print(f"  当前: {format_val(current)}")

        raw = input("  新值: ").strip()
        if raw.lower() == "q":
            break
        if raw == "":
            continue

        try:
            new_val = parse_val(raw, typ)
            if new_val != current:
                set_nested(data, path, new_val)
                changed += 1
                print(f"  -> 已更新: {format_val(new_val)}")
        except (ValueError, TypeError) as e:
            print(f"  -> 格式错误: {e}，跳过")

    if changed:
        bak = PARAM_PATH + ".bak"
        if os.path.exists(PARAM_PATH):
            with open(PARAM_PATH, "r", encoding="utf-8") as f:
                with open(bak, "w", encoding="utf-8") as bf:
                    bf.write(f.read())

        with open(PARAM_PATH, "w", encoding="utf-8") as f:
            json.dump(data, f, indent=2, ensure_ascii=False)
            f.write("\n")

        print(f"\n✓ 已保存 {changed} 项修改到 {PARAM_PATH}")
        print(f"  备份: {bak}")
    else:
        print("\n  没有修改。")


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n已取消。")
