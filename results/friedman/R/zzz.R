

.onLoad <- function(libname, pkgname) 
{
	##	library.dynam(file.path(libname, paste("EAF", .Platform$dynlib.ext, sep="")), pkgname, libname )
	library.dynam("Intervals", pkgname, libname )
}

#.noGenerics <- TRUE

.onUnload <- function(libpath)
    library.dynam.unload("Intervals", libpath)




## If present, .First.lib will be used if the NAMESPACE file is
## missing.  This is useful during development, thanks to C-c C-l in
## Emacs/ESS. It won't be used if a NAMESPACE file is present.  Note:
## Due to registration of C functions done in the NAMESPACE file,
## wireframe (and possibly cloud) won't work in this scenario.


.First.lib <- function(lib, pkg) 
{
    cat(gettext("Note: you shouldn't be seeing this message unless\nyou are using a non-standard version of Intervals"),
        fill = TRUE)
    library.dynam("Intervals", pkg, lib )
##library.dynam(file.path(lib, paste("Intervals", .Platform$dynlib.ext, sep="")), pkg, lib )
}





