#include "gns/gns_service_client.h"
using namespace hrp;
using namespace std;



int main(int argc , char *argv[])
{
  hrp::gns_service_client gns;

  cout<<"updateIP "<<gns.updateIP()<<endl;

  cout<<"Own account Name "<< gns.getOwnAccountName()<<endl;
  cout<<"Own GUID " << gns.getOwnGUID()<<endl;
  cout<<"getGUIDbyAccountName " <<gns.getGUIDbyAccountName("testgamma")<<endl;
  cout<<"getAccountNamebyGUID "<<gns.getAccountNamebyGUID("A2694F1EA7FAF8E917FAEB668267F74912519472")<<endl;


  cout<<"getIPbyGUID----------------------------\n\n";
	  vector<string> ips = gns.getIPbyGUID("A2694F1EA7FAF8E917FAEB668267F74912519472");
    if (! ips.empty()){
      cout<<"\n Obtained these IPs: ";
      for(vector<string>::size_type i = 0; i != ips.size(); i++) {
        cout<<"\t "<<ips[i];
      }
    }
    else
      cout<<"Could not retrieve the GUID to IP";
    cout<<"\n\n----------------------------\n\n";


    cout<<"getIPbyAccountName----------------------------\n\n";
  	  ips = gns.getIPbyAccountName("testgamma");
      if (! ips.empty()){
        cout<<"\n Obtained these IPs: ";
        for(vector<string>::size_type i = 0; i != ips.size(); i++) {
          cout<<"\t "<<ips[i];
        }
      }
      else
        cout<<"Could not retrieve the hostname to IP";

      cout<<"\n\n----------------------------\n\n";



    return 0;
}
