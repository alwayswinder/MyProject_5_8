"""在运行中的 UE 编辑器里执行 Python（走 PythonScriptPlugin 远程执行）。

用法：
    python ue_run.py <脚本文件> [--timeout 秒]

脚本会在编辑器主线程里以 MODE_EXEC_FILE 执行，stdout/stderr 会回传。
"""
import argparse
import os
from pathlib import Path
import sys
import time

sys.path.append(r"E:\Unreal\UE_5.8\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python")
import remote_execution as remote  # noqa: E402

sys.stdout.reconfigure(encoding="utf-8")
sys.stderr.reconfigure(encoding="utf-8")


def connect(timeout=20.0, project=None, node_id=None):
    re = remote.RemoteExecution()
    re.start()
    deadline = time.time() + timeout
    project = os.path.normcase(str(Path(project or Path(__file__).resolve().parents[1]).resolve()))
    nodes = []
    while time.time() < deadline:
        try:
            nodes = [n for n in re.remote_nodes
                     if os.path.normcase(str(Path(n.get('project_root', '')).resolve())) == project
                     and (node_id is None or n.get('node_id') == node_id)]
        except Exception:
            nodes = None
        if nodes:
            break
        time.sleep(0.5)
    try:
        if not nodes:
            raise RuntimeError("没有发现匹配项目的 UE 远程节点: " + project)
        if len(nodes) != 1:
            raise RuntimeError("同一项目有多个 UE 节点，请用 --node-id 指定: " + str(nodes))
        re.open_command_connection(nodes[0]['node_id'])
    except Exception:
        re.stop()
        raise
    return re


def main(argv):
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('script')
    parser.add_argument('--timeout', type=float, default=20.0, help='节点发现超时秒数')
    parser.add_argument('--project', default=str(Path(__file__).resolve().parents[1]))
    parser.add_argument('--node-id')
    args = parser.parse_args(argv[1:])
    with open(args.script, "r", encoding="utf-8-sig") as fh:
        code = fh.read()
    re = connect(args.timeout, args.project, args.node_id)
    try:
        rec = re.run_command(code, exec_mode=remote.MODE_EXEC_FILE, raise_on_failure=False)
    finally:
        re.stop()
    if isinstance(rec, dict):
        out, err, res, ok = rec.get("output"), rec.get("error"), rec.get("result"), rec.get("success")
    else:
        out, err, res, ok = rec.output, None, rec.result, rec.success
    if out:
        print(out)
    if err:
        print("ERR:", err)
    if res:
        print("RESULT:", res)
    if ok:
        print("UE_REMOTE_OK")
        return 0
    print("UE_REMOTE_FAILED")
    return 1


if __name__ == "__main__":
    sys.exit(main(sys.argv))
