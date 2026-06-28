<#
.SYNOPSIS
  PulseSensor STM32U545RE-Q - 命令行构建脚本

.DESCRIPTION
  设置 STM32CubeIDE 工具链环境并编译项目
  编译成功自动生成 HEX/BIN 文件

.EXAMPLE
  .\build.ps1        # 清理并构建
  .\build.ps1 -clean # 仅清理
#>

param(
  [switch]$CleanOnly
)

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path

# 工具链路径（STM32CubeIDE 1.19.0 自带）
$MakePath = "C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.make.win32_2.2.0.202409170845\tools\bin"
$GccPath = "C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.13.3.rel1.win32_1.0.0.202411081344\tools\bin"

# 设置环境变量
$env:Path = "$MakePath;$GccPath;$env:Path"

# 进入项目目录
Set-Location $ProjectDir
Write-Host "=== PulseSensor Build Script ===" -ForegroundColor Cyan
Write-Host "Project:  $ProjectDir" -ForegroundColor Gray
Write-Host "Make:     $((Get-Command make.exe).Source)" -ForegroundColor Gray
Write-Host "Compiler: $((Get-Command arm-none-eabi-gcc.exe).Source)" -ForegroundColor Gray
Write-Host ""

if ($CleanOnly) {
  Write-Host "=== Cleaning project... ===" -ForegroundColor Yellow
  make clean
  if ($LASTEXITCODE -eq 0) {
    Write-Host "Clean completed." -ForegroundColor Green
  }
  exit $LASTEXITCODE
}

# 清理
Write-Host "=== Cleaning... ===" -ForegroundColor Yellow
make clean
Write-Host ""

# 编译
Write-Host "=== Building... ===" -ForegroundColor Green
make all -j4

if ($LASTEXITCODE -eq 0) {
  Write-Host "" -ForegroundColor Green
  Write-Host "=== Build Successful! ===" -ForegroundColor Green

  # 显示固件大小
  Write-Host "" -ForegroundColor Cyan
  arm-none-eabi-size "$ProjectDir\Debug\PulseSensor.elf"

  # 显示输出文件
  Write-Host "" -ForegroundColor Cyan
  Write-Host "Output files:" -ForegroundColor White
  Get-ChildItem "$ProjectDir\Debug\PulseSensor.*" | ForEach-Object {
    Write-Host "  $($_.Name) - $('{0:N0}' -f $_.Length) bytes"
  }
} else {
  Write-Host "" -ForegroundColor Red
  Write-Host "=== Build Failed! ===" -ForegroundColor Red
  exit 1
}