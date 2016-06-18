#include "robot.hpp"

Robot::Robot(std::string hostname, uint32_t port) {
  _p_client = new PlayerCc::PlayerClient(hostname, port);
  _p_proxy  = new PlayerCc::Position2dProxy(_p_client, 0);
  _r_proxy  = new PlayerCc::RangerProxy(_p_client, 0);
  _p_proxy->RequestGeom();
  _r_proxy->RequestGeom();
}

Robot::~Robot() {
  std::cout << "Destroying robot" << std::endl;
  delete _p_proxy;
  delete _r_proxy;
  delete _p_client;
}

IASSS_Robot::IASSS_Robot(std::string hostname, uint32_t port) : Robot(hostname, port) {
  std::cout << "Creating IAS-SS robot on host \"" << hostname << "\" and port " << port << "." << std::endl;
}

IASSS_Robot::~IASSS_Robot() {
  std::cout << "Destroying IAS-SS robot." << std::endl;
}

void IASSS_Robot::run() {
  _p_client->Read();

  if(_r_proxy->GetRange(0) < 0.2) {
    _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(-45));
    sleep(1);
  } else if(_r_proxy->GetRange(_r_proxy->GetRangeCount()) < 0.2) {
    _p_proxy->SetSpeed(0.0f, PlayerCc::dtor(45));
    sleep(1);
  } else {
    _p_proxy->SetSpeed(0.3f, 0.0f);
  }
}
