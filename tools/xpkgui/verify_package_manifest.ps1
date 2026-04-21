param(
	[Parameter(Mandatory = $true)]
	[string] $PackageDir
)

$ErrorActionPreference = 'Stop'

function Get-Sha256Hex([string] $Path) {
	$stream = [System.IO.File]::OpenRead($Path)
	try {
		$sha256 = [System.Security.Cryptography.SHA256]::Create()
		try {
			$hashBytes = $sha256.ComputeHash($stream)
			return ( $hashBytes | ForEach-Object { $_.ToString('x2') } ) -join ''
		} finally {
			$sha256.Dispose()
		}
	} finally {
		$stream.Dispose()
	}
}

$root = ( Resolve-Path -LiteralPath $PackageDir ).Path
$manifestPath = Join-Path $root 'PACKAGE_MANIFEST.txt'

if ( -not ( Test-Path -LiteralPath $manifestPath -PathType Leaf ) ) {
	Write-Error "Missing package manifest: $manifestPath"
	exit 1
}

$entries = 0
Get-Content -LiteralPath $manifestPath |
	ForEach-Object {
		if ( $_ -notmatch '^([0-9a-f]{64})\s+([0-9]+)\s+(.+)$' ) {
			return
		}

		$expectedHash = $Matches[1]
		$expectedSize = [Int64] $Matches[2]
		$relativePath = $Matches[3]
		$fullPath = Join-Path $root ( $relativePath -replace '/', '\' )

		if ( -not ( Test-Path -LiteralPath $fullPath -PathType Leaf ) ) {
			Write-Error "Manifest entry is missing: $relativePath"
			exit 1
		}

		$file = Get-Item -LiteralPath $fullPath
		if ( $file.Length -ne $expectedSize ) {
			Write-Error "Manifest size mismatch: $relativePath"
			exit 1
		}

		$actualHash = Get-Sha256Hex $fullPath
		if ( $actualHash -ne $expectedHash ) {
			Write-Error "Manifest hash mismatch: $relativePath"
			exit 1
		}

		$script:entries += 1
	}

if ( $entries -le 0 ) {
	Write-Error "Package manifest contains no file entries: $manifestPath"
	exit 1
}

Write-Host "Verified $entries package manifest entries."
