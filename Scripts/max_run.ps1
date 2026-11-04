param(
    [Parameter(Mandatory = $true)][string]$ScriptFile,
    [int]$MaxPid = 0,
    [int]$TimeoutMs = 120000
)

# 把 maxscript 文件内容发到运行中的 3ds Max（3dsmax-mcp native bridge，命名管道）。
# 用法：pwsh -File Scripts\max_run.ps1 -ScriptFile Scripts\probe_animcraft_brow.ms

$ErrorActionPreference = 'Stop'

if ($MaxPid -eq 0) {
    $proc = Get-Process 3dsmax -ErrorAction SilentlyContinue | Sort-Object StartTime -Descending | Select-Object -First 1
    if (-not $proc) { throw '3ds Max is not running' }
    $MaxPid = $proc.Id
}

$pipeName = "3dsmax-mcp-pid-$MaxPid"
$code = Get-Content -LiteralPath $ScriptFile -Raw -Encoding UTF8

$payload = @{ type = 'maxscript'; command = $code } | ConvertTo-Json -Compress -Depth 4

$pipe = New-Object System.IO.Pipes.NamedPipeClientStream('.', $pipeName, [System.IO.Pipes.PipeDirection]::InOut)
$pipe.Connect(5000)
$sw = New-Object System.IO.StreamWriter($pipe)
$sw.AutoFlush = $true
$sr = New-Object System.IO.StreamReader($pipe)
$sw.WriteLine($payload)

$task = $sr.ReadLineAsync()
if (-not $task.Wait($TimeoutMs)) {
    $pipe.Dispose()
    throw "Timeout waiting for 3ds Max response ($TimeoutMs ms)"
}
$resp = $task.Result
$pipe.Dispose()

if (-not $resp) { throw 'Empty response from 3ds Max' }

$obj = $resp | ConvertFrom-Json
if ($obj.meta) {
    Write-Host ("META: transport={0} durationMs={1} safeMode={2}" -f $obj.meta.transport, $obj.meta.durationMs, $obj.meta.safeMode)
}
if (-not $obj.success) {
    Write-Host 'MAX_ERROR:' $obj.error
    exit 1
}
Write-Host '--- RESULT ---'
$obj.result
