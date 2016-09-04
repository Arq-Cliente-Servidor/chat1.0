// Coded by: Sebastian Duque Restrepo - Carolina Gomez Trejos
#include <SFML/Audio.hpp>
#include <cassert>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <zmqpp/zmqpp.hpp>

#include "Util/Serializer.hpp"

using namespace std;
using namespace zmqpp;

vector<string> tokenize(string &input) {
  stringstream ss(input);
  vector<string> result;
  string s;
  while (ss >> s)
    result.push_back(s);
  return result;
}

message record(const string &act, const string &friendName,
               bool isRecord = true) {
  message msg;
  sf::SoundBufferRecorder recorder;
  unsigned int sampleRate = 44100;

  if (isRecord) {
    cout << "Press enter to record" << endl;
    cin.ignore(10000, '\n');
    recorder.start(sampleRate);

    cout << "Recording... press enter to stop" << endl;
    cin.ignore(10000, '\n');
    recorder.stop();
    cout << "The voice message has been sent" << endl;
  } else {
    recorder.start(sampleRate);
    this_thread::sleep_for(chrono::milliseconds(500));
    recorder.stop();
  }

  const sf::SoundBuffer &buffer = recorder.getBuffer();
  const int16_t *samples = buffer.getSamples();
  int sampleCount = buffer.getSampleCount();
  vector<int16_t> buffer_msg(samples, samples + sampleCount);
  int channelCount = buffer.getChannelCount();

  msg << act << friendName << buffer_msg << sampleCount << channelCount
      << sampleRate;
  return msg;
}

void recordCallSend(bool &onPlay, const string act, const string friendName,
                    socket &s) {
  while (onPlay) {
    message msg = record(act, friendName, false);
    s.send(msg);
  }
}

bool soundCapture(vector<string> &tokens, socket &s) {
  if (tokens.size() > 1 and
      (tokens[0] == "recordTo" or tokens[0] == "recordGroup")) {
    message msg = record(tokens[0], tokens[1]);
    s.send(msg);
  }
  return false;
}

// Recibir sound como parametro por referencia desde el main a play
void play(sf::Sound &mysound, sf::SoundBuffer &sb, vector<int16_t> &samples,
          int sampleCount, int channelCount, const unsigned int sampleRate, bool isCall = false) {

  int16_t *buffer = &samples[0];

  if (!sb.loadFromSamples(buffer, sampleCount, channelCount, sampleRate)) {
    cout << "Problems playing sound" << endl;
    return;
  }

  if (!isCall) {
    mysound.setBuffer(sb);
    cout << "Press enter to play the voice message" << endl;
    cin.ignore(1, '\n');
    mysound.play();
  } else {
    mysound.setBuffer(sb);
    mysound.play();
  }
}

void receive(message &rep) {
  string senderName;
  rep >> senderName;
  string textContent;
  rep >> textContent;
  cout << "*" << senderName << " says: " << textContent << endl;
}

void recordReceive(message &rep, sf::Sound &mysound, sf::SoundBuffer &sb, bool isCall = false) {
  string senderName;
  rep >> senderName;
  vector<int16_t> samples;
  rep >> samples;
  int sampleCount;
  rep >> sampleCount;
  int channelCount;
  rep >> channelCount;
  int sampleRate;
  rep >> sampleRate;
  play(mysound, sb, samples, sampleCount, channelCount, sampleRate, isCall);
}

void groupReceive(message &rep) {
  string groupName;
  rep >> groupName;
  string senderName;
  rep >> senderName;
  string text;
  rep >> text;
  cout << "[" << groupName << "] " << senderName << " says: " << text << endl;
}

void recordReceiveGroup(message &rep, sf::Sound &mysound, sf::SoundBuffer &sb) {
  string groupName;
  rep >> groupName;
  string senderName;
  rep >> senderName;
  vector<int16_t> samples;
  rep >> samples;
  int sampleCount;
  rep >> sampleCount;
  int channelCount;
  rep >> channelCount;
  int sampleRate;
  rep >> sampleRate;
  cout << "[" << groupName << "] " << senderName << " records to you" << endl;
  play(mysound, sb, samples, sampleCount, channelCount, sampleRate);
}

void callRequest(message &rep, socket &s) {
  string friendName;
  rep >> friendName;
  string txt;
  rep >> txt;
  cout << friendName << txt << endl;
  string acc;
  message req;

  while (getline(cin, acc) and acc != "y" and acc != "n") {
    cout << "Please select a valid option" << endl;
  }

  if (acc == "y") {
    req << "accept" << friendName << true;
  } else {
    req << "accept" << friendName << false;
  }
  s.send(req);
}

void callResponse(message &rep, string &name, bool &onPlay, thread *recorder, socket &s) {
  string friendName;
  rep >> friendName;
  string txt;
  rep >> txt;
  bool callReady;
  rep >> callReady;
  cout << friendName << txt << endl;
  name = friendName;

  if (callReady) {
    onPlay = true;
    string action = "calling";
    cout << "calling to " << friendName << endl;
    recorder = new thread(recordCallSend, ref(onPlay), action, friendName, ref(s));

  } else {
    cout << "The call could not be performed" << endl;
  }
}

void stop(message &rep, bool &onPlay) {
  if (!onPlay) {
    cout << "There is not an active call." << endl;
  } else {
    onPlay = false;
    string friendName;
    rep >> friendName;
    cout << "The call with " << friendName << " has finished" << endl;
  }
}

void addGroup(message &rep) {
  string text;
  rep >> text;
  string groupName;
  rep >> groupName;
  cout << text << "[" << groupName << "]" << endl;
}

bool addFriend(message &rep) {
  string name;
  rep >> name;
  cout << name << " and you are friends now" << endl;
}

bool warning(message &rep) {
  string txt;
  rep >> txt;
  cout << "[INFO]: " << txt << endl;
  bool ok;
  rep >> ok;
  return ok;
}

bool attends(message &rep, sf::Sound &mysound, sf::SoundBuffer &sb, socket &s,
             thread *recorder, bool &onPlay, string &name) {
  string act;
  rep >> act;

  if (act == "receive") {
    receive(rep);
  } else if (act == "callReceive") {
    recordReceive(rep, mysound, sb, true);
  } else if (act == "groupReceive") {
    groupReceive(rep);
  } else if (act == "recordReceive") {
    recordReceive(rep, mysound, sb);
  } else if (act == "recordReceiveGroup") {
    recordReceiveGroup(rep, mysound, sb);
  } else if (act == "callRequest") {
    callRequest(rep, s);
  } else if (act == "callResponse") {
    callResponse(rep, name, onPlay, recorder, s);
  } else if (act == "stop") {
    stop(rep, onPlay);
  } else if (act == "addGroup") {
    addGroup(rep);
  } else if (act == "addFriend") {
    addFriend(rep);
  } else if (act == "warning") {
    return warning(rep);
  }
  return true;
}

int main(int argc, char *argv[]) {
  // TODO no haga llamada si ya esta en una
  // no hacer llamada si no es tu amigo
  // llamada grupal

  if (argc != 5) {
    cerr << "Invalid arguments" << endl;
    return EXIT_FAILURE;
  }

  string address(argv[1]);
  string action(argv[2]);
  string userName(argv[3]);
  string password(argv[4]);
  string sckt("tcp://");
  sckt += address;

  bool onPlay = false;
  sf::SoundBuffer sb;
  sf::Sound mysound;
  thread *recorder = nullptr;
  string friendName;

  context ctx;
  socket s(ctx, socket_type::xrequest);

  if (action != "login" and action != "register") {
    cerr << "invalid operation, usage: <address> <action> <username> <password>"
         << endl;
    return EXIT_FAILURE;
  }

  cout << "Connecting to: " << sckt << endl;
  s.connect(sckt);

  message entry;
  entry << action << userName << password;
  s.send(entry);

  int console = fileno(stdin);
  poller poll;
  poll.add(s, poller::poll_in);
  poll.add(console, poller::poll_in);

  while (true) {
    if (poll.poll()) { // There are events in at least one of the sockets
      if (poll.has_input(s)) {
        // Handle input in socket
        message msg;
        s.receive(msg);
        if (!attends(msg, mysound, sb, s, recorder, onPlay, friendName)) break;
      }
      if (poll.has_input(console)) {
        // Handle input from console
        string input;
        getline(cin, input);
        vector<string> tokens = tokenize(input);
        if (tokens[0] == "stop") {
          onPlay = false;
          message m;
          m << "stop" << friendName;
          cout << "The call with " << friendName << " has finished" << endl;
          s.send(m);
        } else if (tokens[0] == "login")  {
          cout << "You had already logged!" << endl;
        } else if (!soundCapture(tokens, s)) {
          message msg;
          for (const auto &str : tokens) {
            msg << str;
          }
          s.send(msg);
        }
      }
    }
  }

  if (recorder != nullptr) recorder->join();
  return EXIT_SUCCESS;
}
