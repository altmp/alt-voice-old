package("rnnoise")
    if is_plat("windows") then
        add_urls("https://github.com/altmp/rnnoise-cmake/releases/download/$(version)/rnnoise-windows.zip")
        add_versions("f75e7dd", "abfa90ce7b1fa8c048f86075b26fb1045a21655391902e19274059f32ed8b5aa")
    end

    if is_plat("linux") then
        add_urls("https://github.com/altmp/rnnoise-cmake/releases/download/$(version)/rnnoise-linux.zip")
        add_versions("f75e7dd", "844a7c63b3045c186e7c3d89abdbfac753f047360def152179ad812882dbfee6")
    end

    on_install("windows", function (package)
        os.cp("include/rnnoise.h", package:installdir("include"))
        os.cp("lib/release/rnnoise.lib", package:installdir("lib"))
    end)

    on_install("linux", function (package)
        os.cp("include/rnnoise.h", package:installdir("include"))
        os.cp("lib/rnnoise.a", package:installdir("lib"))
    end)