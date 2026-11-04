"""在运行中的 UE 编辑器里执行 Python（走 PythonScriptPlugin 远程执行）。

用法：
    python ue_run.py <脚本文件> [--timeout 秒]

脚本会在编辑器主线程里以 MODE_EXEC_FILE 执行，stdout/stderr 会回传。
"""
import sys
import time

sys.path.append(r"E:\Unreal\UE_5.8\Engine\Plugins\Experimental\PythonScriptPlugin\Content\Python")
import remote_execution as remote  # noqa: E402

sys.stdout.reconfigure(encoding="utf-8")
sys.stderr.reconfigure(encoding="utf-8")


def connect(timeout=20.0):
    re = remote.RemoteExecution()
    re.start()
    deadline = time.time() + timeout
    nodes = None
    while time.time() < deadline:
        try:
            nodes = re.remote_nodes
        except Exception:
            nodes = None
        if nodes:
            break
        time.sleep(0.5)
    if not nodes:
        raise RuntimeError("没有发现 UE 远程执行节点（编辑器是否开启了 Python 远程执行？）")
    try:
        re.open_command_connection(nodes)
    except Exception as exc:  # 老版本接口
        print("open with nodes failed: %r" % (exc,), file=sys.stderr)
        re.open_command_connection("127.0.0.1")
    return re


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 2
    path = argv[1]
    with open(path, "r", encoding="utf-8") as fh:
        code = fh.read()
    re = connect()
    rec = re.run_command(code, exec_mode=remote.MODE_EXEC_FILE, raise_on_failure=False)
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
