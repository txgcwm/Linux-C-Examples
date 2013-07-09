docs/libinifile.ebuild - Gentoo ebuild file for libinifile.

    1. Copy the ebuild into Gentoo portage overlay (I use 
       /usr/local/portage/dev-libs/libinifile)
       
    2. Rename to match current version of libinifile:
    
       bash$> ( cd /usr/local/portage/dev-libs/libinifile
                mv libinifile.ebuild libinifile-x.y.z.ebuild )
		
    3. Update manifest by running:
    
       bash$> ( cd /usr/local/portage/dev-libs/libinifile
                ebuild libinifile-x.y.z.ebuild digest )
		
    4. Install using portage:
    
       bash$> emerge -a libinifile
       
       
// Anders Lövgren, 2008-06-23
