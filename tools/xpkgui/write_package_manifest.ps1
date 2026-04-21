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

$lines = @(
	'xpkgui package manifest',
	"Generated: $(( Get-Date ).ToString('yyyy-MM-ddTHH:mm:ssK'))",
	"Root: $root",
	'',
	'SHA256  Size  Path'
)

Get-ChildItem -LiteralPath $root -File -Recurse |
	Where-Object { $_.FullName -ne $manifestPath } |
	Sort-Object FullName |
	ForEach-Object {
		$relativePath = $_.FullName.Substring($root.Length + 1).Replace('\', '/')
		$hash = Get-Sha256Hex $_.FullName
		$lines += '{0}  {1}  {2}' -f $hash, $_.Length, $relativePath
	}

Set-Content -LiteralPath $manifestPath -Value $lines -Encoding ASCII
