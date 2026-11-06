(
  local out = ""
  try (
    local dib = gw.getViewportDib()
    dib.filename = "D:\\Download\\max_viewport.png"
    save dib
    out += "saved " + ((getFileSize "D:\\Download\\max_viewport.png") as string) + "\n"
  ) catch ( out += "ERR " + (getCurrentException()) + "\n" )
  out
)
