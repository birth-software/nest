Set-StrictMode -Version Latest
Set-PSDebug -Trace 2

$LLVM_VERSION="19.1.2"
$BASE_DOWNLOAD_URL="https://github.com/llvm/llvm-project/releases/download/llvmorg-$LLVM_VERSION"

$LLVM_DOWNLOAD_FILE_BASENAME="clang+llvm-$LLVM_VERSION-x86_64-pc-windows-msvc"
$LLVM_DOWNLOAD_FILE="$LLVM_DOWNLOAD_FILE_BASENAME.tar.xz"
$LLVM_DOWNLOAD_URL="$BASE_DOWNLOAD_URL/$LLVM_DOWNLOAD_FILE"

Invoke-WebRequest -Uri "$LLVM_DOWNLOAD_URL" -OutFile "$LLVM_DOWNLOAD_FILE"
7z x $LLVM_DOWNLOAD_FILE
7z x "$LLVM_DOWNLOAD_FILE_BASENAME.tar"
dir
