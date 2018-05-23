#include "gazebo.hh"
#include "plugins/CameraPlugin.hh"
#include "common/common.hh"
#include "transport/transport.hh"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

// ICE utils includes
#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <easyiceconfig/EasyIce.h>

#include <jderobot/camera.h>

#include <visionlib/colorspaces/colorspacesmm.h>

#include <iostream>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <jderobot/ir.h>


void *mainIR(void* v);

namespace gazebo
{   

  
  class IRD
    {
        public:
            float received;
    };

  class IR : public CameraPlugin
  { 
    public: IR() : CameraPlugin(),count(0) 
	{
		pthread_mutex_init (&mutex, NULL);
		n = round(rand()*1000);
		threshold = 150.0;
		brightness = 255;
	}

    public: void Load(sensors::SensorPtr _parent, sdf::ElementPtr _sdf)
    {
      // Don't forget to load the camera plugin
      CameraPlugin::Load(_parent,_sdf);
      std::cout << "Load: " <<  this->parentSensor->Camera()->Name()<< std::endl;
    } 

    // Update the controller
    public: void OnNewFrame(const unsigned char *_image, 
        unsigned int _width, unsigned int _height, unsigned int _depth, 
        const std::string &_format)
    {
		if(count==0){

			std::string name = this->parentSensor->Camera()->Name();
		
			std::vector<std::string> strs;
			boost::split(strs, name, boost::is_any_of("::"));
		


			nameIR = std::string("--Ice.Config=" + strs[4] + ".cfg");
			// std::cout << "@@@@@@@@@@@@@@@@@@@@ " << nameCamera << std::endl;
			nameCamera = std::string(strs[4]);
			// std::cout << "@@@@@@@@@@@@@@@@@@@@ " << nameCamera << std::endl;

			if (count == 0){
				pthread_t thr_gui;
				pthread_create(&thr_gui, NULL, &mainIR, (void*)this);
			}

			image.create(_height, _width, CV_8UC3);
			count++;
		}
					count++;

		pthread_mutex_lock (&mutex);
        memcpy((unsigned char *) image.data, &(_image[0]), _width*_height * 3);

        cv::cvtColor(image, gray_image, CV_BGR2GRAY);
        brightness = cv::mean(gray_image)[0];
// cv::imshow(nameCamera, image);
// cv::waitKey(0);
					
		float received = 0;
		if (brightness <= threshold)
			received = 1;
		else
			received = 0;

		
// std::cout << brightness << std::endl;
// result = gray_image.clone();
// std::string output = "/tmp/images/"+std::to_string(brightness)+"_R"+std::to_string(received)+".jpg";
// cv::imwrite( output, result );

		IRD data;
		data.received = received;

		irData = data;
		pthread_mutex_unlock (&mutex); 
		
    }         

    private: int count;
    private: int n;
    public: std::string nameIR, nameCamera;
    public: cv::Mat image, gray_image, result;
	public: pthread_mutex_t mutex;
	public: std::string nameGlobal;
	public: float brightness, threshold;
	public: IRD irData;
  };

  // Register this plugin with the simulator
  GZ_REGISTER_SENSOR_PLUGIN(IR)
}

	class IRI: virtual public jderobot::IR {
		public:
			IRI (gazebo::IR* ir) 
			{
				this->ir = ir;
			}

			virtual ~IRI(){};

			virtual jderobot::IRDataPtr getIRData(const Ice::Current&) {
			    jderobot::IRDataPtr irData (new jderobot::IRData());
				pthread_mutex_lock (&ir->mutex); 
				irData->received = ir->irData.received;				
				pthread_mutex_unlock (&ir->mutex); 
				return irData;
			};

		private:
			float received;
			gazebo::IR* ir;
	};

	void *mainIR(void* v) 
	{

		gazebo::IR* ir = (gazebo::IR*)v;
		
		char* name = (char*)ir->nameIR.c_str();
		char* irname = (char*)ir->nameCamera.c_str();

	    Ice::CommunicatorPtr ic;
	    int argc = 1;

	    Ice::PropertiesPtr prop;
		char* argv[] = {name};
	    try {
	        
	        ic = EasyIce::initialize(argc, argv);
	        prop = ic->getProperties();
	        
	        std::string Endpoints = prop->getProperty("IR.Endpoints");
	        std::cout << irname << " Endpoints > " << Endpoints << std::endl;
	        
	        Ice::ObjectAdapterPtr adapter =
	            ic->createObjectAdapterWithEndpoints("IR", Endpoints);
	        Ice::ObjectPtr object = new IRI(ir);
	        adapter->add(object, ic->stringToIdentity(irname));
	        adapter->activate();
	        ic->waitForShutdown();
	    } catch (const Ice::Exception& e) {
	        std::cerr << e << std::endl;
	    } catch (const char* msg) {
	        std::cerr << msg << std::endl;
	    }
	    if (ic) {
	        try {
	            ic->destroy();
	        } catch (const Ice::Exception& e) {
	            std::cerr << e << std::endl;
	        }
	    }

	    return NULL;
	}

// class CameraI: virtual public jderobot::Camera {
// 	public:
// 		CameraI(std::string propertyPrefix, gazebo::IR* camera)
// 			   : prefix(propertyPrefix), cameraI(camera) {
		
// 			imageDescription = (new jderobot::ImageDescription());
// 			cameraDescription = (new jderobot::CameraDescription());
// 			cameraDescription->name = "camera Introrob";

//         	replyTask = new ReplyTask(this);
// 		    replyTask->start(); // my own thread
		  
// 		}

// 		std::string getName () {
// 			return (cameraDescription->name);
// 		}

// 		std::string getRobotName () {
// 			return "RobotName";
// 		}

// 		virtual ~CameraI() {


// 		}

// 		virtual jderobot::ImageDescriptionPtr getImageDescription(const Ice::Current& c){
// 			return imageDescription;
// 		}

// 		virtual jderobot::CameraDescriptionPtr getCameraDescription(const Ice::Current& c){
// 			return cameraDescription;
// 		}

// 		virtual Ice::Int setCameraDescription(const jderobot::CameraDescriptionPtr &description, const Ice::Current& c) {
// 			return 0;
// 		}

// 		virtual void getImageData_async (const jderobot::AMD_ImageProvider_getImageDataPtr& cb,const std::string& format, const Ice::Current& c){
// 			replyTask->pushJob(cb, format);
// 		}

// 		virtual jderobot::ImageFormat getImageFormat(const Ice::Current& c)
// 		{
// 			jderobot::ImageFormat mFormats;
// 			mFormats.push_back(colorspaces::ImageRGB8::FORMAT_RGB8.get()->name);

// 			return mFormats;
// 		}

// 		virtual std::string startCameraStreaming(const Ice::Current&){
//             return "";  // Remove return warning
// 		}

// 		virtual void stopCameraStreaming(const Ice::Current&) {

// 		}

// 		virtual void reset(const Ice::Current&)
// 		{
// 		}

// 	private:
// 		class ReplyTask: public IceUtil::Thread {
// 			public:
// 				ReplyTask(CameraI* camera){
//                     mycamera = camera;
// 				}

// 				void pushJob(const jderobot::AMD_ImageProvider_getImageDataPtr& cb, std::string format){

// 					mFormat = format;
// 					IceUtil::Mutex::Lock sync(requestsMutex);
// 					requests.push_back(cb);
// 				}

// 				virtual void run(){
// 					jderobot::ImageDataPtr reply(new jderobot::ImageData);
// 					struct timeval a, b;
// 					int cycle = 48;
// 					long totalb,totala;
// 					long diff;
					
// 					int count =0 ;

// 					while(1){
						
// 						if(!mycamera->cameraI->image.data){
// 							usleep(100);
// 							continue;
// 						}
// 						if(count==0){
// 							pthread_mutex_lock (&mycamera->cameraI->mutex);
// 							mycamera->imageDescription->width = mycamera->cameraI->image.cols;
// 							mycamera->imageDescription->height = mycamera->cameraI->image.rows;
// 							mycamera->imageDescription->size = mycamera->cameraI->image.cols*mycamera->cameraI->image.rows*3;
// 							pthread_mutex_unlock (&mycamera->cameraI->mutex);

// 							mycamera->imageDescription->format = "RGB8";

// 							reply->description = mycamera->imageDescription;
// 							count++;
// 						}
									
// 						gettimeofday(&a,NULL);
// 						totala=a.tv_sec*1000000+a.tv_usec;


// 						IceUtil::Time t = IceUtil::Time::now();
// 						reply->timeStamp.seconds = (long)t.toSeconds();
// 						reply->timeStamp.useconds = (long)t.toMicroSeconds() - reply->timeStamp.seconds*1000000;
          				
//           				pthread_mutex_lock (&mycamera->cameraI->mutex);
// 					    reply->pixelData.resize(mycamera->cameraI->image.rows*mycamera->cameraI->image.cols*3);
					    
// 					    memcpy( &(reply->pixelData[0]), (unsigned char *) mycamera->cameraI->image.data, mycamera->cameraI->image.rows*mycamera->cameraI->image.cols*3);
// 						pthread_mutex_unlock (&mycamera->cameraI->mutex);

// 					   { //critical region start
// 						   IceUtil::Mutex::Lock sync(requestsMutex);
// 						   while(!requests.empty()) {
// 							   jderobot::AMD_ImageProvider_getImageDataPtr cb = requests.front();
// 							   requests.pop_front();
// 							   cb->ice_response(reply);
// 						   }
// 					   } //critical region end
                  
// 						gettimeofday(&b,NULL);
// 						totalb=b.tv_sec*1000000+b.tv_usec;

// 						diff = (totalb-totala)/1000;
// 						diff = cycle-diff;

// 						if(diff < 33)
// 							diff = 33;


// 						/*Sleep Algorithm*/
// 						usleep(diff*1000);
// 					}
// 				}

// 				CameraI* mycamera;
// 				IceUtil::Mutex requestsMutex;
// 				std::list<jderobot::AMD_ImageProvider_getImageDataPtr> requests;
// 				std::string mFormat;
// 		};

// 		typedef IceUtil::Handle<ReplyTask> ReplyTaskPtr;
// 		std::string prefix;

// 		colorspaces::Image::FormatPtr imageFmt;
// 		jderobot::ImageDescriptionPtr imageDescription;
// 		jderobot::CameraDescriptionPtr cameraDescription;
// 		ReplyTaskPtr replyTask;
// 		gazebo::IR* cameraI;	
		
// }; // end class CameraI


// void *myMain(void* v)
// {

// 	gazebo::IR* ir = (gazebo::IR*)v;

// 	char* name = (char*)camera->nameConfig.c_str();


//     Ice::CommunicatorPtr ic;
//     int argc = 1;

//     Ice::PropertiesPtr prop;
// 	char* argv[] = {name};

//     try {
        
//         ic = EasyIce::initialize(argc, argv);
//         prop = ic->getProperties();
        
//         std::string Endpoints = prop->getProperty("CameraGazebo.Endpoints");
//         std::cout << "CameraGazebo "<< camera->nameCamera <<" Endpoints > "  << Endpoints << std::endl;

//         Ice::ObjectAdapterPtr adapter =
//         ic->createObjectAdapterWithEndpoints("CameraGazebo", Endpoints);
		
//         Ice::ObjectPtr object = new CameraI(std::string("CameraGazebo"),  camera);
//         adapter->add(object, ic->stringToIdentity(camera->nameCamera));
//         adapter->activate();
//         ic->waitForShutdown();
//     } catch (const Ice::Exception& e) {
//         std::cerr << e << std::endl;
//     } catch (const char* msg) {
//         std::cerr << msg << std::endl;
//     }
//     if (ic) {
//         try {
//             ic->destroy();
//         } catch (const Ice::Exception& e) {
//             std::cerr << e << std::endl;
//         }
//     }

//     return NULL;    
// }