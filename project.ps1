Set-StrictMode -Version Latest
Set-PSDebug -Trace 2
$previous_error_action_preference = $global:ErrorActionPreference
$global:ErrorActionPreference = 'Stop'
$myargs=$args
$build_dir="build"
$release_mode="Debug"
$build_type_prefix="build_type="
$cmake_prefix_path_prefix="-DCMAKE_PREFIX_PATH="
$cmake_prefix_path=""

& {
    try
    {
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

        Write-Output "Build type: $release_mode"

        New-Item -Path $build_dir -ItemType Directory -Force
        cmake . "-B$build_dir" -G Ninja "-DCMAKE_BUILD_TYPE=$release_mode" -DCMAKE_C_COMPILER="clang" -DCMAKE_CXX_COMPILER="clang++" "-DCMAKE_PREFIX_PATH=$cmake_prefix_path"
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
