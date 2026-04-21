param(
	[string] $OutputPath = ( Join-Path $PSScriptRoot 'obj\release_audit_x64.md' )
)

$ErrorActionPreference = 'Stop'

$scriptDir = $PSScriptRoot
$rootDir = [System.IO.Path]::GetFullPath(( Join-Path $scriptDir '..\..' ))
$releaseDir = Join-Path $rootDir 'release\x64'
$expected = @(
	'xpkgui.exe',
	'xpkgui_shext.dll',
	'xpkcon.exe',
	'xpack.exe',
	'xpack.dll'
)

if ( -not ( Test-Path -LiteralPath $releaseDir -PathType Container ) ) {
	Write-Error "Missing release directory: $releaseDir"
	exit 1
}

$outputDir = Split-Path -Parent $OutputPath
if ( $outputDir -and -not ( Test-Path -LiteralPath $outputDir -PathType Container ) ) {
	New-Item -ItemType Directory -Path $outputDir | Out-Null
}

$tracked = @{}
try {
	& git -C $rootDir ls-files release/x64 | ForEach-Object {
		$name = [System.IO.Path]::GetFileName($_)
		if ( $name ) {
			$tracked[$name.ToLowerInvariant()] = $true
		}
	}
} catch {
}

$items = Get-ChildItem -LiteralPath $releaseDir -Force |
	Sort-Object Name |
	ForEach-Object {
		$isExpected = $expected -contains $_.Name
		$isTracked = $tracked.ContainsKey($_.Name.ToLowerInvariant())
		[pscustomobject]@{
			Name = $_.Name
			Type = if ( $_.PSIsContainer ) { 'directory' } else { 'file' }
			Expected = $isExpected
			Tracked = $isTracked
			Recommendation = if ( $isExpected ) {
				'keep'
			} elseif ( $isTracked ) {
				'review tracked history before deleting'
			} else {
				'delete before release or keep out of package'
			}
		}
	}

$lines = @(
	'# xpkgui release/x64 audit',
	'',
	"Generated: $(( Get-Date ).ToString('yyyy-MM-ddTHH:mm:ssK'))",
	"Release directory: $releaseDir",
	'',
	'| Item | Type | Expected | Git tracked | Recommendation |',
	'| --- | --- | --- | --- | --- |'
)

foreach ( $item in $items ) {
	$lines += '| {0} | {1} | {2} | {3} | {4} |' -f $item.Name, $item.Type, $item.Expected, $item.Tracked, $item.Recommendation
}

$unexpectedCount = @( $items | Where-Object { -not $_.Expected } ).Count
$trackedUnexpectedCount = @( $items | Where-Object { -not $_.Expected -and $_.Tracked } ).Count

$lines += ''
$lines += "Unexpected items: $unexpectedCount"
$lines += "Tracked unexpected items: $trackedUnexpectedCount"
$lines += ''
$lines += 'Policy: this report is advisory only. Do not delete tracked release files without an explicit release-cleanup decision.'

Set-Content -LiteralPath $OutputPath -Value $lines -Encoding ASCII
Write-Host "Release audit report written: $OutputPath"
