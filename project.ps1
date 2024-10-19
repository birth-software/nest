Set-StrictMode -Version Latest
Set-PSDebug -Trace 2
$previous_error_action_preference = $global:ErrorActionPreference
$myargs=$args
$build_dir="build"
$release_mode="Debug"
$build_type_prefix="build_type="

& {
    try
    {
        $global:ErrorActionPreference = 'Stop'

        if ($($myargs.Length) -ne 0 -and $myargs[0].StartsWith($build_type_prefix))
        {
            $release_mode = $myargs[0].Substring($build_type_prefix.Length)
        }

        Write-Output "Build type: $release_mode"

        New-Item -Path $build_dir -ItemType Directory -Force
        cmake . "-B$build_dir" -G Ninja "-DCMAKE_BUILD_TYPE=$release_mode" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++"
        pushd $build_dir
        ninja
        popd

        if ($($myargs.Length) -ne 0)
        {
            & ".\$build_dir\runner" $myargs
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
}
