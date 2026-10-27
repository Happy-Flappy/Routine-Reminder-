#ifndef ACTOR_H
#define ACTOR_H

//Balloon notifications aren't working!

class Actor
{
	public:
	ws::Icon icon;
	ws::Text text;
	ws::Font font;
	ws::Window window;
	ws::Window trayWindow;
	
	ws::Sprite sprite;
	ws::Sprite speech;
	ws::Sprite buttonYes,buttonNo;
	
	ws::Texture texture;
	ws::Texture speechTex;
	ws::Texture buttonTex;
	
	ws::Vec2i size = {0,0};
	bool wasAwake = false;
	bool sleeping = true;
	bool gotYes = false;
	
	ws::ClickMenu menu;
	ws::TrayIcon tray;
	
	int offset = 0;
	
	Actor(std::string path,ws::Vec2i size)
	{
		this->size = size;
		font.loadFromSystem("Comic Sans");
		if(!texture.loadFromFile(path))
		{
			MessageBoxA(NULL,"Failed to load actor spritesheet. Quitting...","Routine Reminder",MB_OK | MB_ICONINFORMATION);
			exit(0);
		}
		speechTex.loadFromFile("ASSETS/speech.png");
		speech.setTexture(speechTex);
		
		ws::Monitor mon = ws::Screen::getPrimaryMonitor();
		window.create(1920,1080,"",WS_POPUP,WS_EX_TOOLWINDOW);
		window.enableChromaKey(ws::Hue::transparent,true);
		window.setFullscreen();
		
		trayWindow.create(1,1,"",WS_POPUP,WS_EX_TOOLWINDOW);
		trayWindow.setPosition(-100,-100);
		
		float scaleY = window.getSize().y / mon.getBounds().height;
		int taskbar = (mon.getBounds().height - mon.getWorkArea().height) * scaleY;
		taskbar *= 0.8;
		
		sprite.setTexture(texture);
		sprite.setTextureRect({0,0,size.x,size.y});
		sprite.setPosition(window.getSize().x * 0.73,window.getSize().y - taskbar - size.y);
		
		speech.setOrigin(ws::Vec2i(speech.getTextureRect().width/2,speech.getTextureRect().height/2));
		speech.setPosition(window.getSize().x * 0.8,window.getSize().y * 0.5);
		
		text.setFont(font);
		text.setCharacterSize(40);
		text.setFillColor(ws::Hue::black);
		
		if(!buttonTex.loadFromFile("ASSETS/buttons.png"))
		{
			MessageBoxA(NULL,"Failed to load button spritesheet. Quitting...","Routine Reminder",MB_OK | MB_ICONINFORMATION);
			exit(0);			
		}
		
		buttonYes.setTexture(buttonTex);
		buttonYes.setTextureRect({0,0,180,130});
		buttonYes.setOrigin(buttonYes.getTextureRect().width/2,buttonYes.getTextureRect().height/2);
		buttonYes.setPosition(window.getSize().x * 0.7,window.getSize().y * 0.7);
		buttonNo.setTexture(buttonTex);
		buttonNo.setTextureRect({180,0,180,130});
		buttonNo.setOrigin(buttonNo.getTextureRect().width/2,buttonNo.getTextureRect().height/2);
		buttonNo.setPosition(window.getSize().x * 0.9,window.getSize().y * 0.7);
		
		window.setLayerAfter(HWND_TOPMOST);
		
		menu.setList({"Toggle Hide Sleeping","Edit Task List","Restart All Tasks","Quit Program"});
		menu.init(window);
		
		icon.loadFromFile("ASSETS/icons8-quest-96.ico",ICON_BIG);
		tray.init(trayWindow,"Routine Reminder",icon);
		
		offset = -(window.getSize().x * 0.4);
		sprite.move(offset,0);
		speech.move(offset,0);
		buttonYes.move(offset,0);
		buttonNo.move(offset,0);
		text.move(offset,0);
		
	}
	
	void question(std::string msg,int tone)
	{
		std::string add = "";
		ws::Text test = text;
		size_t a = 0;
		while(a < msg.size())
		{
			std::string cand = add + msg[a];
			test.setString(cand);
			
			if(msg[a] != '\n' && !add.empty() && test.getBounds().width > 340)
			{
				size_t lineStart = add.rfind('\n');
				lineStart = (lineStart == std::string::npos) ? 0 : lineStart + 1;
				
				size_t sp = add.rfind(' ');
				
				if(sp == std::string::npos || sp < lineStart)
					add += '\n';
				else
				{
					std::string tail = add.substr(sp + 1);
					add.erase(sp);
					add += '\n';
					add += tail;
				}
				continue;
			}
			
			add += msg[a];
			++a;
		}
		
		text.setString(add);
		text.setOrigin(text.getBounds().width/2,text.getBounds().height/2);			
		text.setPosition(window.getSize().x * 0.8,window.getSize().y * 0.5);
		text.move(offset,0);

		sprite.setTextureRect({size.x * (tone+1),0,size.x,size.y});
		sleeping = false;		
	}

	void hoverButtons()
	{
		ws::Vec2i MPosition = window.toWorld(ws::Global::getMousePos());
		float dx = MPosition.x - buttonYes.getPosition().x;
		float dy = MPosition.y - buttonYes.getPosition().y;
		float dist = std::sqrt(dx * dx + dy * dy);
		if(dist < 50)
			buttonYes.setScale({1.2,1.2});
		else
			buttonYes.setScale({1,1});

		dx = MPosition.x - buttonNo.getPosition().x;
		dy = MPosition.y - buttonNo.getPosition().y;
		dist = std::sqrt(dx * dx + dy * dy);

		if(dist < 50)
			buttonNo.setScale({1.2,1.2});
		else
			buttonNo.setScale({1,1});	
		
		
	}
	
	void updateButtons()
	{
		hoverButtons();
		if(buttonYes.getScale().x != 1 && ws::Global::getButton(ws::Mouse::Left))
		{
			gotYes = true;
			//sprite.setTextureRect({size.x * 8,0,size.x,size.y});
			//startSleepDelay = 
			sprite.setTextureRect({0,0,size.x,size.y});
			sleeping = true;
			wasAwake = true;
			return;
		}
		if(buttonNo.getScale().x != 1 && ws::Global::getButton(ws::Mouse::Left))
		{
			gotYes = false;
			sleeping = true;
			sprite.setTextureRect({0,0,size.x,size.y});
			wasAwake = true;
			return;
		}
	}
	
	
	bool hideWhenInactive = false;
	void update(std::vector<Reminder> &reminders,int current)
	{
		trayWindow.isOpen();
		trayWindow.clear();
		trayWindow.display();
		trayWindow.setPosition(-100,100);
		
		if(tray.isPressed(ws::Mouse::Left) || tray.isPressed(ws::Mouse::Right))
		{
			menu.openAtScreen(ws::Global::getMousePos());
			int res = menu.getResult();

			if(res != 0)
			{
				sprite.setTextureRect({0,0,size.x,size.y});
				sleeping = true;
				wasAwake = true;
				gotYes = false;
			}
			
			if(res == 1)//toggle hide sleeping
			{
				hideWhenInactive = !hideWhenInactive;
				std::string v = (hideWhenInactive) ? "Invisible." : "Visible.";
				std::string message = "Toggled Sleeping to " + v;
				
				ws::Balloon(window,message,"Routine Reminder",icon);
			}
			if(res == 2)//edit task list
			{
				std::string folder = "";
				ws::ResolveRelativePath(folder);
				std::wstring wfolder = ws::toUTF16(folder);
				
				std::string file = "source.txt";
				std::wstring wfile = ws::toUTF16(file);
				
				HINSTANCE result = ShellExecute(NULL,L"open",wfile.c_str(),NULL,wfolder.c_str(),SW_SHOWNORMAL);
				
				if((INT_PTR)result <= 32)
					ws::Balloon(window,"Failed to open tasks source file called (sources.txt)","Routine Reminder",icon);
			}
			if(res == 3)//Restart All Tasks
			{
				
				reminders.clear();
				reminders.push_back({0,0,"DAILY","RESTART_ROUTINE"});
				current = 0;
				
				//save empty state, then get from source - if source is different than this new state, it will use the source.
				if(!saveState(reminders,storedDate,storedWeekStart) || !loadState(reminders,size))
					ws::Balloon(window,"Failed to restart all tasks. Could not locate or edit reminders_state.txt!","Routine Reminder",icon);
				
				sleeping = true;
				sprite.setTextureRect({0,0,size.x,size.y});
				tray.showBalloon("Restarted All Tasks","Routine Reminder");
			}
			if(res == 4)//Quit Program
			{
				saveState(reminders,storedDate,storedWeekStart);
				exit(0);
			}			
		}
		if(!window.isOpen())
			return;
		
		window.clear();

		if(!(sleeping && hideWhenInactive))
		{
			//All drawing.
			window.draw(sprite);
		
			if(!sleeping)//expect an answer - revert to sleeping after receiving an answer.
			{
				updateButtons();
				window.draw(speech);
				window.draw(text);			
				window.draw(buttonYes);
				window.draw(buttonNo);
			}
		}
		window.display();
		window.setLayerAfter(HWND_TOPMOST);
	}
};


#endif