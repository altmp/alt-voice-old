package("bass")
    if is_plat("windows") then
        add_urls("https://www.un4seen.com/files/bass$(version).zip")
        add_versions("24", "eadf39828e0938028a93237fb811efa5cdf16d3670e9658890f948e3f93b8d31")
    end

    if is_plat("linux") then
        add_urls("https://www.un4seen.com/files/bass$(version)-linux.zip")
        add_versions("24", "51bea42d324c5242168b053344a65ed4945715c84f92ed8745e20db33d168ef9")
    end

    on_install("windows", function (package)
        os.cp("c/x64/bass.lib", package:installdir("lib"))
        os.cp("x64/bass.dll", package:installdir("bin"))
        os.cp("c/bass.h", package:installdir("include"))
    end)

    on_install("linux", function (package)
        os.cp("libs/x86_64/libbass.so", package:installdir("bin"))
        os.cp("bass.h", package:installdir("include"))
    end)