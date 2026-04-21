param(
	[string] $SpecPath = ( Join-Path $PSScriptRoot 'XPKGUI_SPEC.md' )
)

$ErrorActionPreference = 'Stop'

if ( -not ( Test-Path -LiteralPath $SpecPath -PathType Leaf ) ) {
	Write-Error "Spec file not found: $SpecPath"
	exit 1
}

$lines = [System.IO.File]::ReadAllLines($SpecPath)
$done = @( $lines | Where-Object { [regex]::IsMatch($_, '^- \[x\] ') } ).Count
$active = @( $lines | Where-Object { [regex]::IsMatch($_, '^- \[~\] ') } ).Count
$todo = @( $lines | Where-Object { [regex]::IsMatch($_, '^- \[ \] ') } ).Count
$remaining = @( $lines | Where-Object { [regex]::IsMatch($_, '^- \[[~ ]\] ') } )

$total = $done + $active + $todo

Write-Host "xpkgui spec status"
Write-Host "Spec: $SpecPath"
Write-Host "Total: $total"
Write-Host "Done: $done"
Write-Host "In progress: $active"
Write-Host "Todo: $todo"

if ( $remaining.Count -gt 0 ) {
	Write-Host ''
	Write-Host 'Remaining items:'
	$remaining | ForEach-Object { Write-Host $_ }
}
