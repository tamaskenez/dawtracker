# DAWTRACKER

An experiment about a novel approach to a DAW software.

The fundamental idea is that early development of DAWs (in the 90s) was heavily influenced by the need to prove that a DAW application is not a toy, it's just as serious as the million dollar studio equipment the professional sound engineers are working with in the studio. Also, the way an analog studio works provided a solid basis for UI decisions for the emerging DAW applications.

However, the way things can work in an analog studio are naturally limited by the physical reality of the hardware equipment. Event though those constraints does not apply in the digital world, I argue that many of those constraints have been unnecessarily transferred into modern DAWs to make them familiar for analog sound engineers.

In this experiment I'm building a DAW without those constraints. The UI decisions are driven by my own frustrations with existing DAWs and my needs as an amateur guitarist who likes to record things at home.

## What is different so far

- Clips are first-class citizens. When you record, you primarily record into a clip, which exists outside of the tracks. You can still set up the recording that the clips end up on a track/tracks just like in a traditional DAW.
- Tracks don't have inserts. Instead, a plugin or a combination of multiple plugins is a named, first-class citizen called a "rig" which exists on its own, outside of tracks. Each clip have an associated rig which they were recorded with. Clips, regardless of their rigs, can be assigned to any track.
- Instead of a single, infinite multitrack tape the timeline consists of individual blocks called "sections" which represent a certain number of bars or seconds of the timeline. The sections can define their own tempo and time signature and clips are attached to different points on these sections. When you move around or delete sections, the attached clips are moving with them.
- The metronome is totally separate from the project, it does not influence the speed of the playback of the project. It's just a metronome you hear when you're recording (without playing back other tracks in the background) and provides a time-base for the recorded clips. When you're recording with playback the other tracks (sections) will define the tempo and time signature of the recorded clips.
