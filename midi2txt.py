from collections import OrderedDict
from mido import MidiFile

def midi_to_music(mid, track_num):
    music = {}

    def add_track(selected_track, track_id):
        time = 0
        for msg in selected_track:
            if msg.time is not None:
                time += msg.time
            if msg.type == "note_on" and msg.velocity > 0:
                note_info = [msg.note, track_id]
                if time in music and note_info not in music[time]:
                    music[time].append(note_info)
                else:
                    music[time] = [note_info]

    if track_num is not None:
        add_track(mid.tracks[track_num - 1], track_num)
    else:
        for i, track in enumerate(mid.tracks):
            add_track(track, i)

    return OrderedDict(sorted(music.items()))

def get_time(music, selected_time):
    key_list = list(music.keys())
    for i, time_value in enumerate(key_list):
        if time_value == selected_time and i + 1 <= len(key_list):
            next_time = key_list[i + 1] if i < len(key_list) - 1 else selected_time
            return next_time - selected_time
    return 0

if __name__ == "__main__":
    file = MidiFile("D:/Music/MIDI Files/登る小さな幼生 - Piano.mid", clip=True)
    music_dict = midi_to_music(file, None)

    # Track for first 2 voices
    first_track = 0
    # Adjust time (Lower = Faster & Higher = Slower)
    tempo_multiplier = 1

    melody_file = open("Melody.txt", "w")
    for tick, notes in music_dict.items():
        note_range = 4 if len(notes) >= 4 else len(notes)
        notes.sort(key=lambda x: x[0], reverse=True)

        note_numbers = ["0", "0", "0", "0"]
        for num in range(note_range):
            note_num = str(notes[num][0])
            note_track = notes[num][1]

            if note_track == first_track:
                if note_numbers[0] == "0":
                    note_numbers[0] = note_num
                elif note_numbers[1] == "0":
                    note_numbers[1] = note_num
            else:
                if note_numbers[2] == "0":
                    note_numbers[2] = note_num
                elif note_numbers[3] == "0":
                    note_numbers[3] = note_num

        melody_file.write("\t".join(note_numbers) + "\t")
        melody_file.write(str(get_time(music_dict, tick) * tempo_multiplier) + "\n")
    melody_file.close()
