# 在脚本所在目录下创建 ClaudeStart.bat 和 CopilotStart.bat

import os

script_dir = os.path.dirname(os.path.abspath(__file__))

CLAUDE_BAT = """\
@echo off
chcp 65001 >nul

cd /d "{path}"

claude --permission-mode bypassPermissions
""".format(path=script_dir)

COPILOT_BAT = """\
@echo off
chcp 65001 >nul

cd /d "{path}"

copilot --allow-all
""".format(path=script_dir)

for filename, content in [("ClaudeStart.bat", CLAUDE_BAT), ("CopilotStart.bat", COPILOT_BAT)]:
    filepath = os.path.join(script_dir, filename)
    with open(filepath, "w", encoding="utf-8") as f:
        f.write(content)
    print(f"已创建: {filepath}")
