/*=============================================================================

  Library: CTK

  Copyright (c) German Cancer Research Center,
    Division of Medical and Biological Informatics

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

=============================================================================*/

#include "ctkXnatTreeBrowserMainWindow.h"
#include "ui_ctkXnatTreeBrowserMainWindow.h"

#include "ctkXnatLoginDialog.h"
#include "ctkXnatTreeModel.h"
#include "ctkXnatSession.h"
#include "ctkXnatDataModel.h"
#include "ctkXnatProject.h"
#include "ctkXnatFile.h"
#include "ctkXnatResource.h"
#include "ctkXnatScan.h"
#include "ctkXnatScanFolder.h"
#include "ctkXnatAssessor.h"
#include "ctkXnatAssessorFolder.h"
#include "ctkXnatReconstruction.h"
#include "ctkXnatReconstructionFolder.h"
#include "ctkXnatSubject.h"
#include "ctkXnatExperiment.h"

ctkXnatTreeBrowserMainWindow::ctkXnatTreeBrowserMainWindow(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::ctkXnatTreeBrowserMainWindow),
  m_Session(0),
  m_TreeModel(new ctkXnatTreeModel())
{
  ui->setupUi(this);

  ui->treeView->setModel(m_TreeModel);
  ui->infoLabel->setText("");

  this->connect(ui->loginButton, SIGNAL(clicked()), SLOT(loginButtonPushed()));
  this->connect(ui->treeView, SIGNAL(clicked(const QModelIndex&)), SLOT(itemSelected(const QModelIndex&)));
  this->connect(ui->downloadButton, SIGNAL(clicked()), SLOT(downloadButtonClicked()));
}

ctkXnatTreeBrowserMainWindow::~ctkXnatTreeBrowserMainWindow()
{
  if (m_Session)
  {
    delete m_Session;
  }
  delete ui;

  delete m_TreeModel;
}

void ctkXnatTreeBrowserMainWindow::loginButtonPushed()
{
  if (m_Session)
  {
    ctkXnatDataModel* dataModel = m_Session->dataModel();
    m_TreeModel->removeDataModel(dataModel);
    ui->treeView->reset();
    delete m_Session;
    m_Session = 0;
    ui->loginButton->setText("Login");
    ui->loginLabel->setText("Disconnected");
    ui->infoLabel->setText("");
  }
  else
  {
    ctkXnatLoginDialog loginDialog;
    if (loginDialog.exec() == QDialog::Accepted)
    {
      m_Session = loginDialog.session();
      if (m_Session)
      {
        ui->loginButton->setText("Logout");
        ui->loginLabel->setText(QString("Connected to %1").arg(m_Session->url().toString()));

        ctkXnatDataModel* dataModel = m_Session->dataModel();
        m_TreeModel->addDataModel(dataModel);
        ui->treeView->reset();
      }
    }
  }
}

void ctkXnatTreeBrowserMainWindow::itemSelected(const QModelIndex &index)
{
  ctkXnatObject* xnatObject = m_TreeModel->xnatObject(index);
  bool downloadable = false;
  downloadable |= dynamic_cast<ctkXnatFile*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatScan*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatScanFolder*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatAssessor*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatAssessorFolder*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatResource*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatReconstruction*>(xnatObject)!=NULL;
  downloadable |= dynamic_cast<ctkXnatReconstructionFolder*>(xnatObject)!=NULL;
  ui->downloadButton->setEnabled(downloadable);
    
    QString info;
    
    ctkXnatProject* proj = dynamic_cast<ctkXnatProject*>(xnatObject);
    if (proj!=NULL)
    {
        info=QString("Project: %1, PI: %2 %3").arg(proj->id(), proj->piFirstName(), proj->piLastName());
    }
    
    ctkXnatSubject* subj = dynamic_cast<ctkXnatSubject*>(xnatObject);
    if (subj!=NULL)
    {
        info=QString("Subject: %1").arg(subj->label());
    }
    
    ctkXnatExperiment* expm = dynamic_cast<ctkXnatExperiment*>(xnatObject);
    if (expm!=NULL)
    {
        info=QString("Experiment: %1").arg(expm->label());
    }
    
    ctkXnatScan* scan = dynamic_cast<ctkXnatScan*>(xnatObject);
    if (scan!=NULL)
    {
        info=QString("Scan: %1 - %2").arg(scan->id(),scan->type());
    }
    
    ui->infoLabel->setText(info);
}

void ctkXnatTreeBrowserMainWindow::downloadButtonClicked()
{
  const QModelIndex index = ui->treeView->selectionModel()->currentIndex();
  QVariant variant = m_TreeModel->data(index, Qt::DisplayRole);
  QString fileName = variant.value<QString>();
  if ( fileName.length() != 0 )
  {
    m_TreeModel->downloadFile(index, fileName);
  }
}
