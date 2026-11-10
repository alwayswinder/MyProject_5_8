# AudioPrep

阿七口型测试的音频准备工具。按需手动运行，独立于 `Tools/FaceMigration/`。

| 文件 | 用途 |
|---|---|
| `extract_aqi.py` | 解码原始 MP3，检测语音并导出最多 6 条候选 WAV |
| `transcribe_aqi.py` | 使用 faster-whisper 识别台词和时间码 |
| `aqi_transcript.json` | 已生成的台词和时间码 |
| `show_transcript.py` | 打印台词和时间码 |
| `export_aqi_lines.py` | 按 `CLIPS` 字典导出指定台词的 WAV |
| `analyze_clip.py` | 检查 `TestAudio/SevenSpeech.wav` 的音量和静音 |

在项目根目录运行，例如：

```powershell
python Tools/AudioPrep/show_transcript.py
python Tools/AudioPrep/export_aqi_lines.py "<ffmpeg.exe 的完整路径>"
python Tools/AudioPrep/analyze_clip.py
```

依赖：音频处理使用 NumPy、soundfile 和 FFmpeg；转写额外需要 faster-whisper，首次使用可能下载模型。显示已有台词仅需 Python 标准库。

`extract_aqi.py` 和 `export_aqi_lines.py` 的 `SRC` 仍指向本机原始 MP3，换电脑或素材时需要修改。前者会在系统临时目录生成 `aqi_pack_16k.f32.wav`；转写读取该文件，导出脚本在缓存不存在时生成该文件。更换素材时应先重新运行提取脚本，避免使用旧缓存。

项目路径根据脚本位置自动定位，不依赖运行时所在目录。WAV 输出仍位于项目根的 `TestAudio/`，转写结果写入本目录的固定文件 `aqi_transcript.json`。重复导出同名 WAV 或重复转写会覆盖对应文件；候选片段采用时间码命名，换素材或改变切分结果后可能留下旧候选，需要按需整理 `TestAudio/candidates/`。
