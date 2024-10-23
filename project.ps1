Set-StrictMode -Version Latest
Set-PSDebug -Trace 2
$previous_error_action_preference = $global:ErrorActionPreference
$global:ErrorActionPreference = 'Stop'
$myargs=$args
$build_dir="build"
$release_mode="Debug"
$build_type_prefix="build_type="
$cmake_prefix_path_prefix="-DCMAKE_PREFIX_PATH="

try
{
    $cmake_prefix_path=""
    foreach ($arg in $myargs)
    {
        if ($arg.StartsWith($build_type_prefix))
        {
            $release_mode = $arg.Substring($build_type_prefix.Length)
        }
        if ($arg.StartsWith($cmake_prefix_path_prefix))
        {
            $cmake_prefix_path = $arg.Substring($cmake_prefix_path_prefix.Length);
        }
    }

    if ($cmake_prefix_path.Equals(""))
    {
        if ($release_mode.Equals("Debug"))
        {
            $cmake_prefix_path="llvm-19.1.2-windows-amd64-msvc17-msvcrt-dbg"
        }
        else
        {
            $cmake_prefix_path="llvm-19.1.2-windows-amd64-msvc17-msvcrt"
        }
    }

    Write-Output "Build type: $release_mode. Prefix $cmake_prefix_path"

    New-Item -Path $build_dir -ItemType Directory -Force
    cmake . "-B$build_dir" -G Ninja "-DCMAKE_BUILD_TYPE=$release_mode" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" "-DCMAKE_PREFIX_PATH=$cmake_prefix_path" "-DCMAKE_VERBOSE_MAKEFILE=ON"
    if ($LastExitCode -ne 0)
    {
        exit $LastExitCode
    }
    pushd $build_dir
    ninja -v
    if ($LastExitCode -ne 0)
    {
        exit $LastExitCode
    }
    popd

    if ($($myargs.Length) -ne 0)
    {
        & ".\$build_dir\runner" $myargs
        if ($LastExitCode -ne 0)
        {
            exit $LastExitCode
        }
    }
}
catch
{
    throw
}
finally
{
    $global:ErrorActionPreference = $previous_error_action_preference
}
