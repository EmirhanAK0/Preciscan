
$path = "C:\Users\Emirhan"
Get-ChildItem -Path $path -Force -ErrorAction SilentlyContinue | ForEach-Object {
    $item = $_
    $size = 0
    try {
        if ($item.PSIsContainer) {
            $size = (Get-ChildItem $item.FullName -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
        } else {
            $size = $item.Length
        }
    } catch {
        $size = 0
    }
    [PSCustomObject]@{
        Name = $item.Name
        "Size(GB)" = [math]::Round($size / 1GB, 2)
        Type = if ($item.PSIsContainer) { "Directory" } else { "File" }
    }
} | Sort-Object "Size(GB)" -Descending | Select-Object -First 20 | Format-Table -AutoSize
