:: "Tools/Windows/glslangValidator.exe" -V shader.vert
:: "Tools/Windows/glslangValidator.exe" -V shader.frag

For /R ./Test %%G IN (*.vert) do (
  "Tools/Windows/glslangValidator.exe" -V "%%G" -o "%%G.spv"
)

For /R ./Test %%G IN (*.frag) do (
  "Tools/Windows/glslangValidator.exe" -V "%%G" -o "%%G.spv"
)
PAUSE