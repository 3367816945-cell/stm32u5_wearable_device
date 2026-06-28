<#
.SYNOPSIS
  PulseSensor STM32U545RE-Q - 命令行烧录脚本

.DESCRIPTION
  使用 STM32CubeProgrammer CLI 通过 SWD 接口烧录固件

.EXAMPLE
  .\flash.ps1  # 烧录固件并复位
#>

$ProjectDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$ElfFile = "$ProjectDir\Debug\PulseSensor.elf"

# STM32CubeProgrammer 路径
$ProgPath = "C:\ST\STM32CubeIDE_1.19.0\STM32CubeIDE\plugins\com.st.stm32cube.ide.mcu.externaltools.cubeprogrammer.win32_2.2.200.202503041107\tools\bin"

Write-Host "=== PulseSensor Flash Script ===" -ForegroundColor Cyan
Write-Host ""

# 检查 ELF 文件是否存在
if (-not (Test-Path $ElfFile)) {
  Write-Host "ERROR: ELF file not found!" -ForegroundColor Red
  Write-Host "  Expected: $ElfFile" -ForegroundColor Gray
  Write-Host "  Run build.ps1 first to compile the project." -ForegroundColor Yellow
  exit 1
}

# 检查烧录工具
$ProgCli = "$ProgPath\STM32_Programmer_CLI.exe"
if (-not (Test-Path $ProgCli)) {
  Write-Host "ERROR: STM32_Programmer_CLI not found!" -ForegroundColor Red
  Write-Host "  Expected: $ProgCli" -ForegroundColor Gray
  Write-Host "  Make sure STM32CubeIDE is installed correctly." -ForegroundColor Yellow
  exit 1
}

# 固件信息
$ElfSize = (Get-Item $ElfFile).Length
Write-Host "Firmware: $ElfFile" -ForegroundColor White
Write-Host "  Size: $('{0:N0}' -f $ElfSize) bytes" -ForegroundColor Gray
Write-Host "Programmer: $ProgCli" -ForegroundColor Gray
Write-Host ""

# 确认
Write-Host "Connect ST-Link and ensure target is powered." -ForegroundColor Yellow
Write-Host "Flashing in 2 seconds..." -ForegroundColor Yellow
Start-Sleep -Seconds 1
Write-Host ""

# 烧录
Write-Host "=== Flashing... ===" -ForegroundColor Green
& $ProgCli -c port=SWD -w "$ElfFile" -v -rst

if ($LASTEXITCODE -eq 0) {
  Write-Host ""
  Write-Host "=== Flash Successful! ===" -ForegroundColor Green
  Write-Host "Target is now running the new firmware." -ForegroundColor White
} else {
  Write-Host ""
  Write-Host "=== Flash Failed! ===" -ForegroundColor Red
  Write-Host "Check:" -ForegroundColor Yellow
  Write-Host "  1. ST-Link is connected" -ForegroundColor White
  Write-Host "  2. Target is powered (3.3V)" -ForegroundColor White
  Write-Host "  3. SWD pins are correct (PA13=SWDIO, PA14=SWCLK)" -ForegroundColor White
  exit 1
}