param(
	[Parameter(Mandatory = $true)]
	[int] $TimeoutSeconds,

	[Parameter(Mandatory = $true)]
	[string] $FilePath,

	[Parameter(ValueFromRemainingArguments = $true)]
	[string[]] $CommandArgs
)

function Quote-CommandArgument([string] $Arg) {
	if ( $null -eq $Arg -or $Arg.Length -eq 0 ) {
		return '""'
	}

	if ( $Arg -notmatch '[\s"]' ) {
		return $Arg
	}

	return '"' + ( $Arg -replace '"', '\"' ) + '"'
}

$psi = New-Object System.Diagnostics.ProcessStartInfo
$psi.FileName = $FilePath
$psi.Arguments = ( $CommandArgs | ForEach-Object { Quote-CommandArgument $_ } ) -join ' '
$psi.WorkingDirectory = ( Get-Location ).Path
$psi.UseShellExecute = $false

try {
	$process = [System.Diagnostics.Process]::Start($psi)
} catch {
	Write-Error "Failed to start command: $FilePath $($psi.Arguments)"
	Write-Error $_.Exception.Message
	exit 125
}

if ( -not $process.WaitForExit($TimeoutSeconds * 1000) ) {
	try {
		$process.Kill()
		$process.WaitForExit()
	} catch {
	}

	Write-Error "Command timed out after $TimeoutSeconds seconds: $FilePath $($psi.Arguments)"
	exit 124
}

exit $process.ExitCode
