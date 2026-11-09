"""离线校验面部迁移文档、名称表、导航与整理后数据保留；不连接或修改 UE / Max。"""
from __future__ import annotations

import argparse
import ast
from collections import Counter
import datetime
import hashlib
import json
from pathlib import Path
import re
import sys
import unicodedata
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / 'Docs'


def sha256(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def headings(text: str) -> set[str]:
    result, seen = set(), Counter()
    fenced = False
    for line in text.splitlines():
        if line.lstrip().startswith(('```', '~~~')):
            fenced = not fenced
        if fenced:
            continue
        match = re.match(r'^#{1,6}\s+(.+?)\s*#*$', line)
        if not match:
            continue
        title = re.sub(r'[`*_]', '', match[1]).lower()
        slug = ''.join(c for c in title if c in '-_' or c.isspace() or unicodedata.category(c)[0] in 'LN')
        slug = slug.replace(' ', '-')
        result.add(slug + ('-' + str(seen[slug]) if seen[slug] else ''))
        seen[slug] += 1
    return result


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', default='Docs/Evidence/DocumentationAudit/validation.json')
    parser.add_argument('--verify-reorganization', action='store_true', help='额外比较本次整理前后的证据、脚本和资产摘要')
    args = parser.parse_args()
    errors: list[str] = []
    existing_script_issues: list[dict[str, str]] = []
    counts: dict[str, int] = {}
    output = (ROOT/args.report).resolve()
    if not output.is_relative_to(ROOT):
        raise ValueError('报告路径必须位于项目目录内')

    def require(condition: bool, message: str) -> None:
        if not condition:
            errors.append(message)

    files = sorted(p for p in DOCS.rglob('*') if p.is_file())
    counts['docs_files'] = len(files)
    require({p.name for p in DOCS.iterdir() if p.is_file()} == {'AGENTS.md', 'migration.config.json'},
            'Docs 顶层只允许 AGENTS.md 和 migration.config.json')
    parsed = {}
    for path in files:
        if path.suffix == '.json':
            try:
                parsed[path] = json.loads(path.read_text(encoding='utf-8-sig'))
            except (ValueError, OSError) as exc:
                errors.append(f'{path.relative_to(ROOT)}: {exc}')
    counts['json_parsed'] = len(parsed)
    manifest_path = DOCS/'Evidence/DocumentationAudit/reorganization.json'
    manifest = parsed.get(manifest_path, {})

    markdown = [*DOCS.rglob('*.md'), *(ROOT/'ProjectNotes').rglob('*.md')]
    links = 0
    for path in markdown:
        text = path.read_text(encoding='utf-8-sig')
        for match in re.finditer(r'!?\[[^\]]*\]\(([^)]+)\)', text):
            value = match[1].strip().strip('<>')
            if re.match(r'[A-Za-z][A-Za-z0-9+.-]*://', value):
                continue
            part, _, anchor = unquote(value).partition('#')
            target = (path.parent / part).resolve() if part else path
            links += 1
            require(target.exists() or target == output, f'{path.relative_to(ROOT)}: 断链 {value}')
            if anchor and target.is_file() and target.suffix == '.md':
                require(anchor in headings(target.read_text(encoding='utf-8-sig')),
                        f'{path.relative_to(ROOT)}: 无效标题锚点 {value}')
        for value in re.findall(r'`((?:Docs|Scripts)/[^`\n]+)`', text):
            # 示例通配符、目录命名模式、组合展示和命令不作为精确文件路径。
            if any(c in value for c in '*{}<> ') or ' / ' in value:
                continue
            if value.endswith(('.md', '.json', '.py', '.ms', '.ps1', '/')):
                require((ROOT/value).exists(), f'{path.relative_to(ROOT)}: 失效内联路径 {value}')
    counts['markdown_files'] = len(markdown)
    counts['local_links'] = links

    def walk(value, origin):
        if isinstance(value, dict):
            for child in value.values():
                walk(child, origin)
        elif isinstance(value, list):
            for child in value:
                walk(child, origin)
        elif isinstance(value, str) and value.startswith('Docs/') and not any(c in value for c in '*{}<> '):
            require((ROOT/value).exists(), f'{origin}: JSON 导航路径不存在 {value}')

    config_path = DOCS/'migration.config.json'
    config = parsed.get(config_path, {})
    if config:
        walk(config, config_path.relative_to(ROOT))
        require(config.get('universal_builder_available') is False, '配置不得宣称已有通用一键构建器')
    reference_path = DOCS/'Characters/Seven.face-controls.json'
    template_path = DOCS/'Templates/CharacterFaceControllers.json'
    reference, template = parsed.get(reference_path, {}), parsed.get(template_path, {})
    rows, blanks = reference.get('bindings', []), template.get('bindings', [])
    expected = config.get('reference_character', {}).get('expected_animation_controls', 72)
    require(len(rows) == len(blanks) == expected, '参考和空模板控制数量不一致')
    for key in ('role_id', 'ue_control', 'max_name'):
        require(len({row[key] for row in rows}) == len(rows), f'Seven {key} 重复')
    require(Counter(row['scope'] for row in rows) == {'requested_or_mirrored': 46, 'existing_base': 26},
            '主清单 / 补充清单数量改变')
    require(all(not row['max_name'] for row in blanks), '空模板不应预填 Max 绑定')
    fields = ('role_id', 'group', 'label', 'side', 'ue_control', 'recipe_doc', 'enabled', 'scope')
    require([tuple(r[k] for k in fields) for r in rows] == [tuple(r[k] for k in fields) for r in blanks],
            '参考实例与空模板的功能定义不一致')
    require([r['max_name'] for r in rows] == [r['reference_max_name'] for r in blanks], '模板示例名与 Seven 不一致')
    require(set(config.get('build_order', [])) == {r['group'] for r in rows}, '搭建分组未完整覆盖名称表')
    require(len(config.get('build_order', [])) == len(set(config.get('build_order', []))), '搭建顺序有重复组')
    for origin, value in ((reference_path, reference), (template_path, template)):
        walk(value, origin.relative_to(ROOT))
        require(value['reference'].get('reuse_numeric_parameters') is False, f'{origin}: 禁止默认复用角色数值')
        for row in value['bindings']:
            require(config.get('recipes', {}).get(row['group']) == row['recipe_doc'],
                    f"{origin}: {row['role_id']} 配方与配置不一致")
    catalog = (DOCS/'Reference/ControllerNames.md').read_text(encoding='utf-8')
    md_template = (DOCS/'Templates/CharacterFaceControllers.md').read_text(encoding='utf-8')
    catalog_lines = [line for line in catalog.splitlines() if line.startswith('|')]
    template_lines = [line for line in md_template.splitlines() if line.startswith('|')]
    for row in rows:
        require(sum(f"`{row['max_name']}`" in line and f"`{row['ue_control']}`" in line for line in catalog_lines) == 1,
                f"名称总表映射不完整或重复: {row['role_id']}")
        require(sum(f"`{row['role_id']}`" in line and f"`{row['max_name']}`" in line for line in template_lines) == 1,
                f"Markdown 模板映射不完整或重复: {row['role_id']}")
    counts['reference_controls'] = len(rows)
    counts['recipe_groups'] = len(config.get('recipes', {}))

    # 检查所有脚本中的 Docs 路径字面量；拼接式路径的根和证据原文件另外由摘要检查保护。
    script_refs, script_count = set(), 0
    for path in (ROOT/'Scripts').glob('*.py'):
        if path.name == Path(__file__).name:
            continue
        script_count += 1
        try:
            tree = ast.parse(path.read_text(encoding='utf-8-sig'))
        except SyntaxError as exc:
            name = path.relative_to(ROOT).as_posix()
            if manifest.get('protected_files', {}).get(name) == sha256(path):
                existing_script_issues.append({'path': name, 'issue': str(exc), 'unchanged_from_before': True})
            else:
                errors.append(f'{name}: Python 语法错误 {exc}')
            continue
        for node in ast.walk(tree):
            if isinstance(node, ast.Constant) and isinstance(node.value, str):
                value = node.value.replace('\\', '/')
                if re.fullmatch(r'Docs/[\w./-]+', value):
                    script_refs.add(value)
                    require((ROOT/value).exists(), f'{path.relative_to(ROOT)}: 脚本数据路径缺失 {value}')
    counts['python_scripts_parsed'] = script_count
    counts['script_docs_paths'] = len(script_refs)

    retained_data, protected_count = 0, 0
    for old, before in (manifest.get('files_before', {}) if args.verify_reorganization else {}).items():
        if old in manifest.get('removed', {}):
            require(not (ROOT/old).exists(), f'已移除文件重新出现: {old}')
            continue
        new = manifest.get('moves', {}).get(old, old)
        path = ROOT/new
        require(path.exists(), f'原文件丢失且未登记移除: {old}')
        if old.startswith('Docs/Evidence/') and old.endswith(('.json', '.txt')) and path.is_file():
            require(sha256(path) == before['sha256'], f'原始证据被改写: {new}')
            retained_data += 1
    for name, before_hash in (manifest.get('protected_files', {}) if args.verify_reorganization else {}).items():
        path = ROOT/name
        require(path.is_file() and sha256(path) == before_hash, f'整理期间源脚本或 UE 资产被改变: {name}')
        protected_count += 1
    # 摘要与语义基准已随文档保留，后续运行不依赖被 Git 忽略的 Saved 备份。
    if args.verify_reorganization:
        require(manifest.get('reference_target_before') == reference.get('target'), 'Seven 目标资产改变')
        binding_fields = ('role_id', 'group', 'label', 'side', 'max_name', 'ue_control', 'enabled', 'scope')
        require([tuple(r[k] for k in binding_fields) for r in manifest.get('reference_bindings_before', [])] ==
                [tuple(r[k] for k in binding_fields) for r in rows], '整理改变了 Seven 功能映射')
    counts['evidence_data_byte_identical'] = retained_data
    counts['protected_scripts_and_assets_byte_identical'] = protected_count

    report = {
        'captured_at': datetime.datetime.now().astimezone().isoformat(),
        'scope': 'Offline documentation and preservation audit only; no editor runtime or visual validation.',
        'verify_reorganization': args.verify_reorganization,
        'passed': not errors, 'counts': counts, 'errors': errors,
        'pre_existing_script_parse_issues': existing_script_issues,
    }
    output.parent.mkdir(parents=True, exist_ok=True)
    output.write_text(json.dumps(report, ensure_ascii=False, indent=2), encoding='utf-8')
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    sys.exit(main())
