package("bass-fx")
    if is_plat("windows") then
        add_urls("https://www.un4seen.com/files/z/0/bass_fx$(version).zip")
        add_versions("24", "71142678cc848f0bbabcfaa18acb7e8fc74543dd11d08521c133a016dd4aa473")
    end

    if is_plat("linux") then
        add_urls("https://www.un4seen.com/files/z/0/bass_fx$(version)-linux.zip")
        add_versions("24", "1ee97610bc2768357c4c344c0d7a058ac95edf51804c391fdcf7644762bd413b")
    end

    on_install("windows", function (package)
        os.cp("c/x64/bass_fx.lib", package:installdir("lib"))
        os.cp("x64/bass_fx.dll", package:installdir("bin"))
        os.cp("c/bass_fx.h", package:installdir("include"))
    end)

    on_install("linux", function (package)
        os.cp("libs/x86_64/libbass_fx.so", package:installdir("bin"))
        os.cp("c/bass_fx.h", package:installdir("include"))
    end)