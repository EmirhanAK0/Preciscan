
$paths = @("C:\Users\Emirhan\Downloads", "C:\Users\Emirhan\Desktop", "C:\Users\Emirhan\OneDrive - Yildiz Technical University")
foreach ($p in $paths) {
    Write-Host "--- Top 10 files in $p ---"
    Get-ChildItem -Path $p -File -Recurse -ErrorAction SilentlyContinue | 
        Sort-Object Length -Descending | 
        Select-Object Name, @{Name="Size(MB)";Expression={[math]::Round($_.Length / 1MB, 2)}}, FullName -First 10 | 
        Format-Table -AutoSize
}
