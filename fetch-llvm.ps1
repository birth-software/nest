Set-StrictMode -Version Latest
Set-PSDebug -Trace 2

$LLVM_VERSION="19.1.2"
$BASE_DOWNLOAD_URL="https://github.com/birth-software/build-llvm/releases/download/llvm-$LLVM_VERSION"

$LLVM_DOWNLOAD_FILE_BASENAME="llvm-$LLVM_VERSION-windows-amd64-msvc17-msvcrt"
if ($args[0].Equals("Debug"))
{
    $LLVM_DOWNLOAD_FILE_BASENAME="$LLVM_DOWNLOAD_FILE_BASENAME-dbg"
}

$LLVM_DOWNLOAD_FILE="$LLVM_DOWNLOAD_FILE_BASENAME.7z"
$LLVM_DOWNLOAD_URL="$BASE_DOWNLOAD_URL/$LLVM_DOWNLOAD_FILE"

Write-Output "URL: $LLVM_DOWNLOAD_URL"

Invoke-WebRequest -Uri "$LLVM_DOWNLOAD_URL" -OutFile "$LLVM_DOWNLOAD_FILE"
if ($LastExitCode -ne 0)
{
    exit $LastExitCode
}
7z x $LLVM_DOWNLOAD_FILE
if ($LastExitCode -ne 0)
{
    exit $LastExitCode
}
dir
