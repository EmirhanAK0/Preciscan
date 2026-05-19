
$path = "C:\Users\Emirhan\AppData\Local\Temp"
if (Test-Path $path) {
    $size = (Get-ChildItem $path -Recurse -File -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
    Write-Host "Temp Folder Size: $([math]::Round($size / 1GB, 2)) GB"
}
