<#
.SYNOPSIS
    Preciscan uygulamasını Release modunda derler, bağımlılıkları (Qt + DLL'ler) toplar ve Inno Setup ile kurulum dosyası oluşturur.
#>

$ProjectRoot = "$PSScriptRoot\.."
$PcDir = "$ProjectRoot\pc"
$DeployDir = "$ProjectRoot\deploy"
$DistDir = "$DeployDir\dist"
$BuildDir = "$PcDir\build_release"

# 1. Temizlik
Write-Host ">>> [1/5] Temizlik yapiliyor..." -ForegroundColor Cyan
if (Test-Path $DistDir) { Remove-Item -Recurse -Force $DistDir }
New-Item -ItemType Directory -Force -Path $DistDir | Out-Null

# 2. CMake ile Release Derleme
Write-Host ">>> [2/5] CMake ile Release modunda derleniyor..." -ForegroundColor Cyan
Set-Location $PcDir
cmake -S . -B $BuildDir -DCMAKE_BUILD_TYPE=Release
if ($LASTEXITCODE -ne 0) { throw "CMake yapilandirma hatasi!" }

cmake --build $BuildDir --config Release
if ($LASTEXITCODE -ne 0) { throw "Derleme hatasi!" }

# 3. Exe ve DLL kopyalama
Write-Host ">>> [3/5] Dosyalar dist klasorune kopyalaniyor..." -ForegroundColor Cyan
$ExePath = "$BuildDir\Release\preciscan_ui.exe"
if (-not (Test-Path $ExePath)) {
    # Bazen Release klasoru olmadan dogrudan build altina atabilir (generator'a bagli)
    $ExePath = "$BuildDir\preciscan_ui.exe"
}
if (-not (Test-Path $ExePath)) { throw "preciscan_ui.exe bulunamadi! ($ExePath)" }

Copy-Item $ExePath -Destination $DistDir
Copy-Item "$PcDir\third_party\microepsilon\LLT.dll" -Destination $DistDir

# 4. Qt Bagimliliklarini Toplama (windeployqt)
Write-Host ">>> [4/5] Qt bagimliliklari toplaniyor (windeployqt)..." -ForegroundColor Cyan
# Sistemde windeployqt.exe'nin PATH uzerinde oldugu varsayilir. (Orn: C:\Qt\6.5.3\msvc2019_64\bin)
$Windeployqt = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($Windeployqt) {
    Set-Location $DistDir
    windeployqt --release --no-translations --no-opengl-sw --compiler-runtime "preciscan_ui.exe"
} else {
    Write-Host "UYARI: windeployqt bulunamadi! Lutfen Qt 'bin' klasorunu PATH'e ekleyin veya Qt Command Prompt kullanin." -ForegroundColor Yellow
}

# 4.5. Arduino CH340 Sürücüsünü İndir ve Ekle
$DriverUrl = "https://cdn.sparkfun.com/assets/learn_tutorials/8/4/4/CH341SER.EXE"
$DriverPath = "$DistDir\CH341SER.EXE"
Write-Host ">>> [4.5/5] Arduino (CH340) surucusu indiriliyor..." -ForegroundColor Cyan
try {
    Invoke-WebRequest -Uri $DriverUrl -OutFile $DriverPath -UseBasicParsing
    Write-Host "Surucu basariyla indirildi." -ForegroundColor Green
} catch {
    Write-Host "UYARI: CH340 surucusu indirilemedi. Lutfen internet baglantinizi kontrol edin." -ForegroundColor Yellow
}

# 5. Inno Setup ile Setup.exe uretme
Write-Host ">>> [5/5] Inno Setup ile kurulum dosyasi uretiliyor..." -ForegroundColor Cyan
$Iscc = "C:\Program Files (x86)\Inno Setup 6\ISCC.exe"
if (Test-Path $Iscc) {
    Set-Location $DeployDir
    & $Iscc "setup.iss"
    Write-Host "BAŞARILI! Kurulum dosyasi 'deploy/Output/' icerisinde hazir." -ForegroundColor Green
} else {
    Write-Host "UYARI: Inno Setup (ISCC.exe) bulunamadi. Lutfen Inno Setup'i kurun ve 'setup.iss' dosyasina cift tiklayarak kendiniz derleyin." -ForegroundColor Yellow
    Write-Host "İndirme linki: https://jrsoftware.org/isdl.php" -ForegroundColor Yellow
}
