%define BINNAME qdcontour
Summary: ‰{BINNAME}
Name: smartmet-%{BINNAME}
Version: 14.4.29
Release: 1%{?dist}.fmi
License: FMI
Group: Development/Tools
URL: http://www.weatherproof.fi
Source0: %{name}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-buildroot-%(%{__id_u} -n)
BuildRequires: boost-devel
BuildRequires: freetype-devel
BuildRequires: glibc-devel
BuildRequires: libjpeg-devel
BuildRequires: libpng-devel
BuildRequires: libsmartmet-imagine2 >= 14.4.29
BuildRequires: libsmartmet-newbase >= 14.3.14
BuildRequires: libsmartmet-tron >= 14.2.21
BuildRequires: zlib-devel
Requires: freetype
Requires: libjpeg
Requires: libpng
Requires: zlib
Requires: xorg-x11-fonts-misc
Provides: qdcontour

%description
FMI qdcontour2

%prep
rm -rf $RPM_BUILD_ROOT

%setup -q -n %{BINNAME}
 
%build
make %{_smp_mflags}
make --quiet test

%install
%makeinstall

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root,0775)
%{_bindir}/qdcontour


%changelog
* Tue Apr 29 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.29-1.fmi
- Created a version based on imagine2
* Mon Apr 28 2014 Tuomo Lauri <tuomo.lauri@fmi.fi> - 14.4.28-1.fmi
- Recompile due to additions to SYKE flood parameters
* Thu Apr 10 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.4.10-1.fmi
- Recompiled to be able to draw new pollen parameters
* Mon Mar 31 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.3.31-1.fmi
- Render label markers also at labelsxy positions
* Mon Mar 17 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.3.17-1.fmi
- Fixed contour cache to use interpolated time instead of valid time in cache key
* Wed Jan 22 2014 Mika Heiskanen <mika.heiskanen@fmi.fi> - 14.1.22-1.fmi
- Meteorological arrows are now rendered by rounding to nearest integer knots
* Wed Nov 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.11.27-1.fmi
- Recompiled to get support for the gnomonic projection
* Fri Sep 27 2013 Mika Heiskanen <mika.heiskanen@fmi.fi> - 13.9.27-1.fmi
- Added windcomponents command for drawing arrows from X/Y or U/V components
* Fri Aug  2 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.8.2-2.fmi
- Bug fix to forcing Atlantic views of Pacific data
* Fri Aug  2 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.8.2-1.fmi
- Enabled Pacific views of data
* Wed Jul  3 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.7.3-1.fmi
- Update to boost 1.54
* Thu Jun 20 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.6.20-1.fmi
- Added loglinear interpolation
* Fri Jun 14 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.6.14-1.fmi
- Added timestampformat command for modifying the output filename
* Thu Apr 18 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.4.18-1.fmi
- Meteorological arrow flags are now filled according to the stroke colour
* Tue Mar 26 2013 mheiskan <mika.heiskanen@fmi.fi> - 13.3.26-1.fmi
- arrowfill and arrowstroke can now be specified for speed ranges
* Tue Oct  2 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.10.2-1.fmi
- New parameter names for ice storage from newbase
* Thu Jul  5 2012 Mika Heiskanen <mika.heiskanen@fmi.fi> - 12.7.5-1.el6.fmi
- Migration to boost 1.50
* Wed Jul  4 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.7.4-1.el6.fmi
- New newbase parameter names for wind fractiles
* Mon May 28 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.5.28-1.el6.fmi
- Parameters can now be specified by their number
* Mon Mar 19 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.19-1.el6.fmi
- New LAPS parameter names now recognized
* Mon Feb 13 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.13-3.el5.fmi
- Ignore fill colour of meteorological wind arrows for all rendering methods
* Mon Feb 13 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.13-2.el5.fmi
- Fixed arrow rendering to check whether the speed value is missing
* Mon Feb 13 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.13-1.el5.fmi
- datareplace now works for arrow rendering too
* Wed Feb  8 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.8-1.el6.fmi
- New parameternames: IceSpeed, IceDirection
* Tue Feb  7 2012 mheiskan <mika.heiskanen@fmi.fi> - 12.2.7-1.el5.fmi
- New parametername Friction
- Fixed wind chill formula
* Fri Nov 25 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.11.25-1.el5.fmi
- fillarrow has no effect on meteorological arrows anymore
* Thu Nov 24 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.11.24-1.el5.fmi
- Upgraded to latest newbase which defines RadarBorder parameter
* Mon Oct 17 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.10.17-1.el5.fmi
- Upgraded to latest newbase
* Wed Oct  5 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.10.5-1.el5.fmi
- Upgraded to latest newbase
* Thu Sep 15 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.9.15-1.el5.fmi
- Returned graticule command which was lost in some cvs merge or something
* Wed Jul 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.7.20-1.el5.fmi
- Upgrade to boost 1.47
* Fri Jul  8 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.7.8-1.el5.fmi
- Recompiled to get new biomass parameter names into use
* Fri May 20 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.20-1.el6.fmi
- RHEL6 release
* Mon Mar 28 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.3.28-2.el5.fmi
- Flipped metorological arrows to get the flags on the correct side
- Fixed meteorological arrow speed values
* Thu Mar 24 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.5.20-1.el5.fmi
- Fixed meteorological arrows
- Upgrade to boost 1.46 and new base libraries
* Thu Mar 17 2011 keranen <pekka.keranen@geosaaga.fi> - 11.3.17-1.el6.fmi
- RHEL6 build with compilation fixes
* Wed Feb  2 2011 mheiskan <mika.heiskanen@fmi.fi> - 11.2.2-1.el5.fmi
- Updated to latest newbase with new parameter names
* Tue Oct 12 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.10.12-1.el5.fmi
- timestampbackground is now by default 185,185,185,185
* Mon Jul  5 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.7.5-1.el5.fmi
- Upgrade to newbase 10.7.5-1
* Mon Jun  7 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.6.7-1.el5.fmi
- Output directories are now created automatically
- Now supports pkj projection via new newbase
* Wed Jan 20 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.20-1.el5.fmi
- Added the "overlay" command
* Fri Jan 15 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.15-1.el5.fmi
- Upgrade to boost 1.41
* Thu Jan  7 2010 mheiskan <mika.heiskanen@fmi.fi> - 10.1.7-1.el5.fmi
- Fixed label rendering (imagine lib)
* Tue Jul 14 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.7.14-1.el5.fmi
- Upgrade to boost 1.39
* Fri Jun 12 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.6.12-1.el5.fmi
- Added datehour timestamp formatting for YLE
* Thu May 28 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.5.28-1.el5.fmi
- Added roundarrow support
* Fri Jan 23 2009 mheiskan <mika.heiskanen@fmi.fi> - 9.1.23-1.el5.fmi
- Recompile for recognizing new newbase parameternames
* Tue Dec 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.12.30-1.el5.fmi
- Added safety checks for polar regions in wind arrow calculations
* Mon Dec 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.12.29-1.el5.fmi
- Fixed wind arrows to calculate north correctly
- Added graticule command
* Wed Oct  8 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.10.8-1.el5.fmi
- Bugfix in tron polygon builder
* Mon Sep 29 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.29-1.el5.fmi
- Newbase headers changed
* Wed Sep 24 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.9.24-1.el5.fmi
- Linked statically with boost 1.36
- Improved regression tests
* Mon May 19 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.5.19-1.el5.fmi
- Added contourlinewidth command
* Fri May 16 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.5.16-1.el5.fmi
- Fixed a memory leak caused by rounding errors in creating meteorological arrows
* Tue Mar 11 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.3.11-1.el5.fmi
- Linked with latest newbase including wind interpolation fixes
* Wed Jan 30 2008 mheiskan <mika.heiskanen@fmi.fi> - 8.1.30-1.el5.fmi
- New roadmodel parameter names are now recognized
* Fri Dec 28 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.6-1.el5.fmi
- Linked with fixed newbase
* Fri Nov 30 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.5-1.el5.fmi
- Linked with newest libraries
* Mon Nov 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.4-1.el5.fmi
- Linked with newest libraries
* Thu Nov 15 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.3-1.el5.fmi
- Linked with newest libraries
* Fri Oct 19 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.2-1.el5.fmi
- Added "contourlabeltext" command.
* Mon Sep 24 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-4.el5.fmi
- Fixed "make depend".
* Fri Sep 14 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-3.el5.fmi
- Added "make tag" feature.
* Thu Sep 13 2007 mheiskan <mika.heiskanen@fmi.fi> - 1.0.1-2.el5.fmi
- Improved make system.
* Thu Jun  7 2007 tervo <tervo@xodin.weatherproof.fi> - 
- Initial build.

