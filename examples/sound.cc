#include <iostream>
#include <SFML/Audio.hpp>
#include <string>
#include <unistd.h>
#include <unordered_map>

using namespace std;

string getPath(string ext) {
  char buff[256];
  getcwd(buff, 256);
  string cwd(buff);
  return cwd + ext;
}

void playSound() {
  string path = getPath("/examples/resources/");
  // Load a sound buffer from a wav file
  sf::SoundBuffer buffer;
  if (!buffer.loadFromFile(path + "canary.wav"))
    return;

  // Display sound informations
  cout << "Canary.wav:" << endl;
  cout << " " << buffer.getDuration().asSeconds() << " seconds"       << endl;
  cout << " " << buffer.getSampleRate()           << " samples / sec" << endl;
  cout << " " << buffer.getChannelCount()         << " channels"      << endl;

  // Create a sound instance and play it
  sf::Sound sound(buffer);
  sound.play();

  // Loop while the sound is playing
  while (sound.getStatus() == sf::Sound::Playing) {
    // Leave some CPU time for other processes
    sf::sleep(sf::milliseconds(100));

    // Display the playing position
    cout << "\rPlaying... " << sound.getPlayingOffset().asSeconds() << " sec        ";
    cout << flush;
  }
  cout << endl << endl;
}

void playMusic(const string &filename) {
  string path = getPath("/examples/resources/");
  // Load an ogg music file
  sf::Music music;
  if (!music.openFromFile(path + filename))
    return;

  // Diplsay music informations
  cout << filename << ":" << endl;
  cout << " " << music.getDuration().asSeconds() << " seconds"       << endl;
  cout << " " << music.getSampleRate()           << " samples / sec" << endl;
  cout << " " << music.getChannelCount()         << " channels"      << endl;

  // Play it
  music.play();

  // Loop while the music is playing
  while (music.getStatus() == sf::Music::Playing) {
    // Leave some CPU time for other processes
    sf::sleep(sf::microseconds(100));

    // Display the playing position
    cout << "\rPlaying... " << music.getPlayingOffset().asSeconds() << " sec        ";
    cout << flush;
  }
  cout << endl << endl;
}

using namespace sf;

int ho(unordered_map<int, pair<Sound, SoundBuffer>> &sounds) {
  string path = getPath("/examples/resources/");
  for (int i = 0; i < 20; ++i) {
    if (!sounds[i].second.loadFromFile(path + "orchestral.ogg")) return -1;
    if ((i & 1) and !sounds[i].second.loadFromFile(path + "canary.wav")) return -1;
    sounds[i].first.setBuffer(sounds[i].second);
    sounds[i].first.play();
  }

  cout << "sali" << endl;
}

int main() {
  // load something into the sound buffer...
  // if (!buffer.loadFromFile("sou.ogg")) return -1;
  unordered_map<int, pair<Sound, SoundBuffer>> sounds;
  ho(sounds);

  int t;
  cin >> t;
  return 0;
}

// int main() {
//   // Play a sound
//   playSound();
//
//   // Play music from an ogg file
//   playMusic("ding.flac");
//   playMusic("orchestral.ogg");
//
//   // Wait until the user presses 'enter' key
//   cout << "Press enter to exit..." << endl;
//   cin.ignore(10000, '\n');
//
//   return EXIT_SUCCESS;
// }
