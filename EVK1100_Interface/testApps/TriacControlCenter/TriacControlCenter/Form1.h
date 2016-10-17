#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "rustlightUpdClient.h"


namespace TriacControlCenter {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	protected:
		static	Form1^  singleton ;
//		RustlightUpdClient* pRustlightUpdClient;
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::NumericUpDown^  addressField;
	protected: 

	private: System::Windows::Forms::Button^  sendButton;
	private: System::Windows::Forms::Button^  quitButton;



	protected: 
		System::Int32 addressValue;
	private: System::Windows::Forms::Label^  adradr;
	private: System::Windows::Forms::NumericUpDown^  ValueUpDown;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::CheckBox^  onCheckBox;
	private: System::Windows::Forms::TextBox^  logText;



	protected: 

	protected: 
	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->addressField = (gcnew System::Windows::Forms::NumericUpDown());
			this->sendButton = (gcnew System::Windows::Forms::Button());
			this->quitButton = (gcnew System::Windows::Forms::Button());
			this->adradr = (gcnew System::Windows::Forms::Label());
			this->ValueUpDown = (gcnew System::Windows::Forms::NumericUpDown());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->onCheckBox = (gcnew System::Windows::Forms::CheckBox());
			this->logText = (gcnew System::Windows::Forms::TextBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->addressField))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ValueUpDown))->BeginInit();
			this->SuspendLayout();
			// 
			// addressField
			// 
			this->addressField->Location = System::Drawing::Point(106, 16);
			this->addressField->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {3, 0, 0, 0});
			this->addressField->Name = L"addressField";
			this->addressField->Size = System::Drawing::Size(120, 20);
			this->addressField->TabIndex = 0;
			this->addressField->Tag = L"";
			// 
			// sendButton
			// 
			this->sendButton->Location = System::Drawing::Point(44, 138);
			this->sendButton->Name = L"sendButton";
			this->sendButton->Size = System::Drawing::Size(75, 23);
			this->sendButton->TabIndex = 1;
			this->sendButton->Text = L"send";
			this->sendButton->UseVisualStyleBackColor = true;
			this->sendButton->Click += gcnew System::EventHandler(this, &Form1::sendButton_Click);
			// 
			// quitButton
			// 
			this->quitButton->Location = System::Drawing::Point(348, 139);
			this->quitButton->Name = L"quitButton";
			this->quitButton->Size = System::Drawing::Size(75, 23);
			this->quitButton->TabIndex = 2;
			this->quitButton->Text = L"quit";
			this->quitButton->UseVisualStyleBackColor = true;
			this->quitButton->Click += gcnew System::EventHandler(this, &Form1::quitButton_Click);
			// 
			// adradr
			// 
			this->adradr->AutoSize = true;
			this->adradr->Location = System::Drawing::Point(41, 16);
			this->adradr->Name = L"adradr";
			this->adradr->Size = System::Drawing::Size(44, 13);
			this->adradr->TabIndex = 3;
			this->adradr->Text = L"address";
			this->adradr->Click += gcnew System::EventHandler(this, &Form1::label1_Click);
			// 
			// ValueUpDown
			// 
			this->ValueUpDown->Location = System::Drawing::Point(106, 52);
			this->ValueUpDown->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {1000, 0, 0, 0});
			this->ValueUpDown->Name = L"ValueUpDown";
			this->ValueUpDown->Size = System::Drawing::Size(120, 20);
			this->ValueUpDown->TabIndex = 4;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(41, 59);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(33, 13);
			this->label1->TabIndex = 5;
			this->label1->Text = L"value";
			this->label1->Click += gcnew System::EventHandler(this, &Form1::label1_Click_1);
			// 
			// onCheckBox
			// 
			this->onCheckBox->AutoSize = true;
			this->onCheckBox->Location = System::Drawing::Point(44, 99);
			this->onCheckBox->Name = L"onCheckBox";
			this->onCheckBox->Size = System::Drawing::Size(38, 17);
			this->onCheckBox->TabIndex = 6;
			this->onCheckBox->Text = L"on";
			this->onCheckBox->UseVisualStyleBackColor = true;
			// 
			// logText
			// 
			this->logText->Location = System::Drawing::Point(44, 168);
			this->logText->Multiline = true;
			this->logText->Name = L"logText";
			this->logText->ReadOnly = true;
			this->logText->ScrollBars = System::Windows::Forms::ScrollBars::Both;
			this->logText->Size = System::Drawing::Size(379, 177);
			this->logText->TabIndex = 7;
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(435, 357);
			this->Controls->Add(this->logText);
			this->Controls->Add(this->onCheckBox);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->ValueUpDown);
			this->Controls->Add(this->adradr);
			this->Controls->Add(this->quitButton);
			this->Controls->Add(this->sendButton);
			this->Controls->Add(this->addressField);
			this->Name = L"Form1";
			this->Text = L"triacControlCenter";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->addressField))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ValueUpDown))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) {
				 singleton = this;
				 this->onCheckBox->Checked = true;

				 RustlightUpdClient::initClass(&addToLog);

				 RustlightUpdClient::rustlightUpdClientSingleton->initRustlightTcpIp("192.168.1.155","10001");
			 }

	private: System::Void sendButton_Click(System::Object^  sender, System::EventArgs^  e) {
				 System::Decimal dec = this->addressField->Value;
				 System::Int32 adr = Decimal::ToInt32(dec);

				 dec = this->ValueUpDown->Value;
				 System::Int32 val = Decimal::ToInt32(dec);

				 System::Boolean onState = this->onCheckBox->Checked;
				 System::Int32 onInt = onState;
/*				 if (onState)  {
					 onInt = 1;
				 }  else {
					 onInt = 0;
				 } */

			
			 }

	private: System::Void quitButton_Click(System::Object^  sender, System::EventArgs^  e) {
				 this->Close();
			 }

	private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) {
			 }

private: System::Void label1_Click_1(System::Object^  sender, System::EventArgs^  e) {
		 }


public:	static System::Void addToLog(const char *emsg, ...)
		{
			va_list ap;
			va_start(ap, emsg);

			singleton->addLogText(emsg, ap);

			va_end(ap);
		}

		#define sbuffersize 1024

public:	System::Void addLogText(const char *emsg, ...)
		{
			va_list ap;
			va_start(ap, emsg);
			char sbuffer[sbuffersize];


		//	const wchar_t* wc = GetWC(emsg);

		//	logText.AppendFormat(wc,ap);
			_snprintf(sbuffer, sbuffersize - 1, emsg, ap);
			sbuffer[sbuffersize] = 0x00;
//			logText.Append(sbuffer);
		//	delete wc;
//			UpdateData(false);
			va_end(ap);
		}


};
}

