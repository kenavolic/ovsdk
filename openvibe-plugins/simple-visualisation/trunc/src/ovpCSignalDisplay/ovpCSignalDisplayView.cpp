#include "ovpCSignalDisplayView.h"


#include <iostream>

#include <sstream>

using namespace OpenViBE;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::SimpleVisualisation;

using namespace OpenViBEToolkit;

using namespace std;

namespace OpenViBEPlugins
{
	namespace SimpleVisualisation
	{
		//! Callback for the close window button
		static void gtk_widget_do_nothing(::GtkWidget* pWidget)
		{
		}

		//! Callback for the ZoomIn button
		void zoomInButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
			{
				//toggle off the other buttons
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[1], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[2], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[3], FALSE);

				//sets the cursor mode to 1 (zoomIn)
				l_pView->m_ui32CurrentCursorMode=1;
			}
			else
			{
				//sets the cursor mode back to normal
				l_pView->m_ui32CurrentCursorMode=0;
			}
		}

		void zoomOutButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
			{
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[0], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[2], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[3], FALSE);
				
				l_pView->m_ui32CurrentCursorMode=2;
			}
			else
			{
				l_pView->m_ui32CurrentCursorMode=0;
			}
		}

		void bestFitButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
			{
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[0], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[1], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[3], FALSE);
				
				l_pView->m_ui32CurrentCursorMode=3;
			}
			else
			{
				l_pView->m_ui32CurrentCursorMode=0;
			}
		}

		void normalSizeButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			if(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget)))
			{
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[0], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[1], FALSE);
				gtk_toggle_tool_button_set_active(l_pView->m_pCursorMode[2], FALSE);

				l_pView->m_ui32CurrentCursorMode=4;
			}
			else
			{
				l_pView->m_ui32CurrentCursorMode=0;
			}
		}

		void toggleLeftRulerButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			l_pView->toggleLeftRulers(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget))?true:false);
		}
		
		void toggleBottomRulerButtonCallback(GtkWidget *widget, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			l_pView->toggleBottomRulers(gtk_toggle_tool_button_get_active(GTK_TOGGLE_TOOL_BUTTON(widget))?true:false);
		}

		gboolean spinButtonValueChangedCallback(GtkSpinButton *widget,  gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);

			//Compute and save the nuew number of buffers to display
			OpenViBE::boolean l_bNumberOfDisplayedBufferChanged = 
					l_pView->m_pSignalDisplayDatabase->adjustNumberOfDisplayedBuffers(
					static_cast<float64>(gtk_spin_button_get_value(GTK_SPIN_BUTTON(widget))));

			if(l_bNumberOfDisplayedBufferChanged)
			{
				//reserve the maximum space needed for computing the points to display
				//(when cropping the lines, there can be up to two times the number of original points)
				l_pView->m_pPoints.reserve((size_t)(l_pView->m_pSignalDisplayDatabase->m_ui32SamplesPerBuffer * l_pView->m_pSignalDisplayDatabase->m_ui64NumberOfBufferToDisplay * 2));

				//resize the vector of raw points (before cropping)
				l_pView->m_pRawPoints.resize((size_t)(l_pView->m_pSignalDisplayDatabase->m_ui32SamplesPerBuffer * l_pView->m_pSignalDisplayDatabase->m_ui64NumberOfBufferToDisplay));

				//Redraw the window
				l_pView->redraw();
			}

			return FALSE;
		}

		//called when the channel select button is pressed (opens the channel selection dialog)
		void channelSelectButtonCallback(GtkButton *button, gpointer data)
		{		
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);
			GtkWidget * l_pChannelSelectDialog = glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayChannelSelectDialog");

			//reset all checkbuttons
			for(size_t i=0 ; i<l_pView->m_vChannelsCheckButtons.size() ; i++)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pView->m_vChannelsCheckButtons[i]), false);
			}

			//sets only channels in the list
			for(size_t i=0 ; i<l_pView->m_vSelectedChannels.size() ; i++)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pView->m_vChannelsCheckButtons[(size_t)l_pView->m_vSelectedChannels[i]]), true);
			}

			//finally, show the information dialog
			gtk_widget_show_all(l_pChannelSelectDialog);
		}

		//Called when the user press the apply button of the channel selection dialog
		void channelSelectDialogApplyButtonCallback(GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);

			//clears the list of selected channels
			l_pView->m_vSelectedChannels.clear();

			for(unsigned int i=0 ; i<l_pView->m_vChannelsCheckButtons.size() ; i++)
			{
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pView->m_vChannelsCheckButtons[i])))
				{
					//if a button is checked, add the corresponding channel to the list of selected ones
					l_pView->m_vSelectedChannels.push_back(i);
					//show the channel's display
					l_pView->showChannel(i);
				}
				else
				{
					//this channel is not selected, hides it
					l_pView->hideChannel(i);
				}
			}

			l_pView->updateMainTableStatus();

			//hides the channel selection dialog
			gtk_widget_hide(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayChannelSelectDialog"));
		}	

		//Called when the user press the Information button (opens the information dialog)
		void informationButtonCallback(GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);

			//gets the different values from the database and updates the corresponding label's text field
			stringstream l_oValueString;
			l_oValueString<<l_pView->m_pSignalDisplayDatabase->m_ui32ChannelCount;
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayNumberOfChannels")),
				 l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pSignalDisplayDatabase->m_ui32SamplingFrequency;	
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplaySamplingFrequency")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pSignalDisplayDatabase->m_ui32SamplesPerBuffer;
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplaySamplesPerBuffer")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pSignalDisplayDatabase->m_f64MinimumValue;
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayMinimumValue")),
				l_oValueString.str().c_str() );

			l_oValueString.str("");
			l_oValueString<<l_pView->m_pSignalDisplayDatabase->m_f64MaximumValue;
			gtk_label_set_text(GTK_LABEL(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayMaximumValue")),
				   l_oValueString.str().c_str() );

			GtkWidget * l_pInformationDialog = glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayInformationDialog");

			//connect the close button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayInformationClose")),
					"clicked",
					G_CALLBACK(gtk_widget_hide),
					G_OBJECT(l_pInformationDialog));

			//finally, show the information dialog
			gtk_widget_show(l_pInformationDialog);
		}

		//called when the channel select button is pressed (opens the channel selection dialog)
		void multiViewButtonCallback(GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);

			GtkWidget * l_pMultiViewDialog = glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayMultiViewDialog");

			//reset all checkbuttons
			for(size_t i=0 ; i<l_pView->m_vMultiViewChannelsCheckButtons.size() ; i++)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pView->m_vMultiViewChannelsCheckButtons[i]), false);
			}

			//sets only channels in the list
			for(size_t i=0 ; i<l_pView->m_vMultiViewSelectedChannels.size() ; i++)
			{
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(l_pView->m_vMultiViewChannelsCheckButtons[(size_t)l_pView->m_vMultiViewSelectedChannels[i]]), true);
			}

			//finally, show the information dialog
			gtk_widget_show_all(l_pMultiViewDialog);
		}

		//Called when the user press the apply button of the channel selection dialog
		void multiViewDialogApplyButtonCallback(GtkButton *button, gpointer data)
		{
			CSignalDisplayView* l_pView = reinterpret_cast<CSignalDisplayView*>(data);

			//clears the list of selected channels
			l_pView->m_vMultiViewSelectedChannels.clear();
			
			for(unsigned int i=0 ; i<l_pView->m_vMultiViewChannelsCheckButtons.size() ; i++)
			{
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(l_pView->m_vMultiViewChannelsCheckButtons[i])))
				{
					//if a button is checked, add the corresponding channel to the list of selected ones
					l_pView->m_vMultiViewSelectedChannels.push_back(i);
				}
			}

			l_pView->changeMultiView();

			l_pView->updateMainTableStatus();

			//hides the channel selection dialog
			gtk_widget_hide(glade_xml_get_widget(l_pView->m_pGladeInterface, "SignalDisplayMultiViewDialog"));
		}	

		CSignalDisplayView::CSignalDisplayView(CSignalDisplayDatabase& oSignalDisplayDatabase)
		: m_pGladeInterface(NULL),
		m_pMainWindow(NULL),
		m_ui32CurrentCursorMode(0),
		m_pSignalDisplayDatabase(&oSignalDisplayDatabase),
		m_bMultiViewInitialized(false)
		{
			//load the glade interface
			m_pGladeInterface=glade_xml_new("../share/openvibe-plugins/simple-visualisation/openvibe-simple-visualisation-SignalDisplay.glade", NULL, NULL);
			
			if(!m_pGladeInterface)
			{
				g_warning("Couldn't load the interface!");
				return;
			}

			glade_xml_signal_autoconnect(m_pGladeInterface);

			//gets the pointers to the cursor mode buttons
			m_pCursorMode[0] = GTK_TOGGLE_TOOL_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayZoomInButton"));
			m_pCursorMode[1] = GTK_TOGGLE_TOOL_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayZoomOutButton"));
			m_pCursorMode[2] = GTK_TOGGLE_TOOL_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayBestFitButton"));
			m_pCursorMode[3] = GTK_TOGGLE_TOOL_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayNormalSizeButton"));

			//creates the cursors
			m_pCursor[0] = gdk_cursor_new(GDK_LEFT_PTR);
			m_pCursor[1] = gdk_cursor_new(GDK_SIZING);
			m_pCursor[2] = gdk_cursor_new(GDK_SIZING);	//TODO change
			m_pCursor[3] = gdk_cursor_new(GDK_SIZING);	//TODO change
			m_pCursor[4] = gdk_cursor_new(GDK_SIZING);	//TODO change

			//connect the callback to the cursor mode buttons
			g_signal_connect(G_OBJECT(m_pCursorMode[0]), "toggled", G_CALLBACK (zoomInButtonCallback), this);
			g_signal_connect(G_OBJECT(m_pCursorMode[1]), "toggled", G_CALLBACK (zoomOutButtonCallback), this);
			g_signal_connect(G_OBJECT(m_pCursorMode[2]), "toggled", G_CALLBACK (bestFitButtonCallback), this);
			g_signal_connect(G_OBJECT(m_pCursorMode[3]), "toggled", G_CALLBACK (normalSizeButtonCallback), this);

			//same for the other buttons
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayToggleLeftRuler")),     "toggled",       G_CALLBACK(toggleLeftRulerButtonCallback),   this);
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayToggleBottomRuler")),   "toggled",       G_CALLBACK(toggleBottomRulerButtonCallback), this);
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectButton")), "clicked",       G_CALLBACK(channelSelectButtonCallback),     this);
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewButton")),     "clicked",       G_CALLBACK(multiViewButtonCallback),         this);
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayInformationButton")),   "clicked",       G_CALLBACK(informationButtonCallback),       this);
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayTimeScale")),           "value-changed", G_CALLBACK(spinButtonValueChangedCallback),  this);

			//channel select dialog's signals
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectApplyButton")), "clicked", G_CALLBACK(channelSelectDialogApplyButtonCallback), this);

			//connect the cancel button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectCancelButton")),
					"clicked",
					G_CALLBACK(gtk_widget_hide),
					G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectDialog")));

			//hides the dialog if the user tries to close it	
			 g_signal_connect (G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectDialog")), 
					 "delete_event", 
					 G_CALLBACK(gtk_widget_hide), NULL);

			//multiview signals
			g_signal_connect(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewApplyButton")), "clicked", G_CALLBACK(multiViewDialogApplyButtonCallback), this);

			//connect the cancel button to the dialog's hide command
			g_signal_connect_swapped(G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewCancelButton")),
				"clicked",
				G_CALLBACK(gtk_widget_hide),
				G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewDialog")));

			//hides the dialog if the user tries to close it	
			 g_signal_connect (G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewDialog")), 
				 "delete_event", 
				 G_CALLBACK(gtk_widget_hide), NULL);

			//does nothing on the main window if the user tries to close it
			g_signal_connect (G_OBJECT(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMainWindow")), 
				"delete_event", 
				G_CALLBACK(gtk_widget_do_nothing), NULL);

			//creates the window
			m_pMainWindow = glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMainWindow");
			gtk_widget_show(m_pMainWindow);

		}

		CSignalDisplayView::~CSignalDisplayView()
		{
			//destroy the window and its children
			if(m_pMainWindow)
			{
				gtk_widget_destroy(m_pMainWindow);
				m_pMainWindow = NULL;
			}
			
			//destroy the rest
			for(int i=0 ; i<5 ; i++)
			{
				gdk_cursor_unref(m_pCursor[i]);
			}
			
			/* unref the xml file as it's not needed anymore */
			g_object_unref(G_OBJECT(m_pGladeInterface));
			m_pGladeInterface=NULL;
		}

		void CSignalDisplayView::changeMultiView()
		{
			//if the multiview isn't initialized
			if(!m_bMultiViewInitialized)
			{
				//Do nothing
			}
			//if there are no channels to display in the multiview
			else if(m_vMultiViewSelectedChannels.empty())
			{
				//hides the multiview display (last one in the list)
				CHANNEL_DISPLAY(m_oChannelDisplay.back())->m_pChannelDisplay->resetChannelList();
				hideChannel(m_oChannelDisplay.size() - 1);
			}
			//there are channels to display in the multiview
			else
			{
				if(!GTK_WIDGET_VISIBLE(GTK_WIDGET(m_pSignalDisplayTable)))
				{
					//if there were no selected channels before, but now there are, show the table again
					gtk_widget_show(GTK_WIDGET(m_pSignalDisplayTable));
				}

				if(!GTK_WIDGET_VISIBLE(GTK_WIDGET((m_oChannelDisplay.back()))))
				{
					showChannel(m_oChannelDisplay.size() - 1);
				}

				//updates channels to display list
				CHANNEL_DISPLAY(m_oChannelDisplay.back())->m_pChannelDisplay->resetChannelList();

				for(size_t i=0 ; i<m_vMultiViewSelectedChannels.size() ; i++)
				{
					CHANNEL_DISPLAY(m_oChannelDisplay.back())->m_pChannelDisplay->addChannel(m_vMultiViewSelectedChannels[i]);
				}

				//request a redraw	
				gdk_window_invalidate_rect(GTK_WIDGET(m_oChannelDisplay.back())->window,
					NULL,
					true);
			}
		}

		void CSignalDisplayView::init()
		{
			OpenViBE::uint32 l_ui32ChannelCount = m_pSignalDisplayDatabase->m_ui32ChannelCount;
			vector<string>& l_vChannelName = m_pSignalDisplayDatabase->m_vChannelName;

			stringstream l_oLabelString;
			GtkWidget * l_pChannelSelectList = glade_xml_get_widget(m_pGladeInterface, "SignalDisplayChannelSelectList");
			GtkWidget * l_pMultiViewSelectList = glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMultiViewSelectList");

			//creates the array of Channel displays
			m_oChannelDisplay.resize(l_ui32ChannelCount);
			m_oChannelLabel.resize(l_ui32ChannelCount);

			//gets the pointer to the channels table
			m_pSignalDisplayTable = glade_xml_get_widget(m_pGladeInterface, "SignalDisplayMainTable");

			//resize this table according to the number of channels
			gtk_table_resize(GTK_TABLE(m_pSignalDisplayTable), (l_ui32ChannelCount+1)*2, 3);

			//sets a minimum size for the table (needed to scroll)
			gtk_widget_set_size_request(m_pSignalDisplayTable, 600, (l_ui32ChannelCount+1)*100);

			//Add a vertical separator
			GtkWidget* l_pSeparator = gtk_vseparator_new();
			gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), l_pSeparator,
				1, 2, 0, (l_ui32ChannelCount+1)*2, GTK_SHRINK, static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), 0, 0);
			gtk_widget_show(l_pSeparator);

			//creates as many ChannelDisplay widget as there are channels and adds them to the table
			for(uint32 i=0 ; i<l_ui32ChannelCount ; i++)
			{
				//adds a label with the channel's name
				if(l_vChannelName[i] == "")
				{
					//If no name has been set, just keep the channel index
					l_oLabelString<<"Channel "<<i;
				}
				else
				{	//else keep the index plus the name
					l_oLabelString<<i<<" : "<<l_vChannelName[i];
				}
				GtkWidget * l_pLabel =  gtk_label_new(l_oLabelString.str().c_str());

				m_oChannelLabel[i] = l_pLabel;

				gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),l_pLabel,
					0, 1, i*2, (i*2)+1,
					GTK_FILL, GTK_SHRINK,
					0, 0);

				//
				gtk_widget_show(l_pLabel);
				
				//creates the channel display widget
				m_oChannelDisplay[i] = GTK_WIDGET(channel_display_new());
					
				//configures it
				CHANNEL_DISPLAY(m_oChannelDisplay[i])->m_pChannelDisplay->setParent(this);
				CHANNEL_DISPLAY(m_oChannelDisplay[i])->m_pChannelDisplay->addChannel(i);
				
				//adds it to the table
				gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), m_oChannelDisplay[i],
				2, 3, (i*2), (i*2)+1,
				static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
				0, 0);
				gtk_widget_show_all(m_oChannelDisplay[i]);
					
					
				//Add an horizontal separator under it
				l_pSeparator = gtk_hseparator_new();
				gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), l_pSeparator,
				0, 3, (i*2)+1, (i*2)+2,	static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), GTK_SHRINK,	0, 0);
				gtk_widget_show(l_pSeparator);
				
				
				//Adds a checkbox in the channel select window
				GtkWidget * l_pChannelCheckButton = gtk_check_button_new_with_label(l_oLabelString.str().c_str());
				m_vChannelsCheckButtons.push_back(l_pChannelCheckButton);
				
				gtk_box_pack_start_defaults(GTK_BOX(l_pChannelSelectList), l_pChannelCheckButton);
								
				//same for the multiview dialog
				l_pChannelCheckButton = gtk_check_button_new_with_label(l_oLabelString.str().c_str());
				m_vMultiViewChannelsCheckButtons.push_back(l_pChannelCheckButton);
				gtk_box_pack_start_defaults(GTK_BOX(l_pMultiViewSelectList), l_pChannelCheckButton);

				l_oLabelString.str("");

				//a channel is selected by default
				m_vSelectedChannels.push_back(i);
			}

			//The multiview channel
			//sets a minimum size for the table (needed to scroll)

			GtkWidget * l_pLabel =  gtk_label_new("Multi-View");

			m_oChannelLabel.push_back(l_pLabel);

			gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable),l_pLabel,
				0, 1, l_ui32ChannelCount*2, (l_ui32ChannelCount*2)+1,
				GTK_FILL, GTK_SHRINK,
				0, 0);

			//creates the channel display widget
			m_oChannelDisplay.push_back(GTK_WIDGET(channel_display_new()));

			//adds it to the table
			gtk_table_attach(GTK_TABLE(m_pSignalDisplayTable), m_oChannelDisplay.back(),
				2, 3, (l_ui32ChannelCount*2), (l_ui32ChannelCount*2)+1,
				static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL), 
				static_cast<GtkAttachOptions>(GTK_EXPAND | GTK_FILL),
				0, 0);

			CHANNEL_DISPLAY(m_oChannelDisplay.back())->m_pChannelDisplay->setParent(this);
			CHANNEL_DISPLAY(m_oChannelDisplay.back())->m_pChannelDisplay->addChannel(0);

			gtk_widget_show_all(m_oChannelDisplay.back());
			gtk_widget_hide(m_oChannelDisplay.back());

			m_bMultiViewInitialized = true;

			//reserve the maximum space needed for computing the points to display
			//(when cropping the lines, there can be up to two times the number of original points)
			m_pPoints.reserve((size_t)(m_pSignalDisplayDatabase->m_ui32SamplesPerBuffer*m_pSignalDisplayDatabase->m_ui64NumberOfBufferToDisplay * 2));

			//resize the vector of raw points
			m_pRawPoints.resize((size_t)(m_pSignalDisplayDatabase->m_ui32SamplesPerBuffer*m_pSignalDisplayDatabase->m_ui64NumberOfBufferToDisplay));
			//Used if the initial number of displayed buffer is fixed, computes the corresponding time interval
			//gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayTimeScale")), 
			//		static_cast<gdouble>(m_pSignalDisplayDatabase->getDisplayedTimeIntervalWidth() ) ); 

			gtk_spin_button_set_value(GTK_SPIN_BUTTON(glade_xml_get_widget(m_pGladeInterface, "SignalDisplayTimeScale")), 1000); 

			//finally, show the window
			gtk_widget_show(m_pMainWindow);
		}

		void CSignalDisplayView::redraw()
		{
			//nothing to redraw if the table isn't visible
			if(m_pSignalDisplayTable && GTK_WIDGET_VISIBLE(m_pSignalDisplayTable))
			{
				boolean l_bNotInitialized = false;
				//update the signal scaling/translating parameters if in bestsize/normal mode
				for(uint32 i=0 ; i<m_oChannelDisplay.size()&& !l_bNotInitialized; i++)
				{
					if(m_oChannelDisplay[i])
					{
						if(GTK_WIDGET_VISIBLE(GTK_WIDGET(m_oChannelDisplay[i])))
						{
							CHANNEL_DISPLAY(m_oChannelDisplay[i])->m_pChannelDisplay->updateSignalZoomParameters();
						}
					}
					else
					{
						l_bNotInitialized = true;
					}
				}

				if(!l_bNotInitialized && m_pSignalDisplayDatabase->m_ui32ChannelCount!=0)
				{
					//The channels table needs to be redrawn
					gdk_window_invalidate_rect(GTK_WIDGET(m_pSignalDisplayTable)->window,
							NULL,
							true);
				}
			}
		}

		void CSignalDisplayView::toggleLeftRulers(OpenViBE::boolean bActive)
		{
			for(size_t i=0 ; i<m_oChannelDisplay.size() ; i++)
			{
				CHANNEL_DISPLAY(m_oChannelDisplay[i])->m_pChannelDisplay->toggleLeftRuler(bActive);
			}
		}

		void CSignalDisplayView::toggleBottomRulers(OpenViBE::boolean bActive)
		{
			for(size_t i=0 ; i<m_oChannelDisplay.size() ; i++)
			{
				CHANNEL_DISPLAY(m_oChannelDisplay[i])->m_pChannelDisplay->toggleBottomRuler(bActive);
			}
		}

		void CSignalDisplayView::hideChannel(OpenViBE::uint32 ui32ChannelIndex)
		{
			gtk_widget_hide( m_oChannelDisplay[ui32ChannelIndex] );
			gtk_widget_hide( m_oChannelLabel[ui32ChannelIndex] );
		}

		void CSignalDisplayView::showChannel(OpenViBE::uint32 ui32ChannelIndex)
		{
			gtk_widget_show( m_oChannelDisplay[ui32ChannelIndex] );
			gtk_widget_show( m_oChannelLabel[ui32ChannelIndex] );
		}

		void CSignalDisplayView::updateMainTableStatus()
		{
			//if nothing has been selected
			if(m_vSelectedChannels.empty() && m_vMultiViewSelectedChannels.empty())
			{
				//hide the whole table
				gtk_widget_hide(GTK_WIDGET(m_pSignalDisplayTable));
			}
			else
			{
				if(!GTK_WIDGET_VISIBLE(GTK_WIDGET(m_pSignalDisplayTable)))
				{
					//if there were no selected channels before, but now there are, show the table again
					gtk_widget_show(GTK_WIDGET(m_pSignalDisplayTable));
				}
			}
		}
	}
}
