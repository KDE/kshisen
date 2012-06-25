// Version numbers are of the form: MAJOR.MINOR.MICRO
// The MAJOR number should only be incremented when really major changes occur.
// The MINOR number is increased to a release if new features were added.
// The MICRO version is increased for every bugfix release.
//
// The MICRO version number is only used in release branches. Trunk/Master uses
// the first two numbers of the current release with a '+' sign to indicate the
// development state.
//
// In trunk a commit number (see commit.h) indicates the sub version. The
// commit number is increased absolutely and not reset on releases.
//
// Example: Version "1.8.3" is the current stable version, so the trunk
//          version number is "1.8+" with the commit number "11" resulting
//          in the version info "1.8+ #11".
//          With every substantial change in the code the commit number
//          is increased.
//          With no new features but two changes at release time:
//            - stable version is "1.8.4"
//            - trunk version is "1.8+ #13"
//          With new features and two changes at release time:
//            - stable version is "1.9"
//            - trunk version is "1.9+ #13"

#ifndef KSHISEN_VERSION
#define KSHISEN_VERSION "1.8.5"
#endif // KSHISEN_VERSION
