#include "networkwidget.h"
#include "ui_networkwidget.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLineEdit>
#include <QMessageBox>
#include <QNetworkReply>
#include <QPushButton>
#include <QRegularExpression>
#include <QStackedLayout>
#include <QUrlQuery>

NetworkWidget::NetworkWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::NetworkWidget)
{
    ui->setupUi(this);
    this->setWindowTitle("视频搜索工具");
    this->setWindowIcon(QIcon(":/res/seek.png"));
    this->setWindowFlag(Qt::WindowStaysOnTopHint);

    this->manager=new QNetworkAccessManager(this);
    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    search="https://search.bilibili.com/all";//B站的搜索视频api

    connect(ui->comboBox,&QComboBox::currentTextChanged,this,[=](QString text){
        if(text=="BiliBili"){
            this->website=1;
            search="https://search.bilibili.com/all";//B站的搜索视频api
        }else if(text=="QQ音乐"){
            this->website=2;
            search="https://y.qq.com/n/ryqq/search";//旧api
            // search="https://c.y.qq.com/soso/fcgi-bin/client_search_cp";//qq音乐的搜索api
        }else if(text=="酷狗音乐"){
            this->website=3;
            // search="https://www.kugou.com/yy/index.php?r=play/search";//酷狗音乐的搜索api
            search="https://complexsearch.kugou.com/v2/search/song"; // 新版搜索接口
        }
    });

    connect(ui->btn_seek,&QPushButton::clicked,this,[=](){
        QString key=ui->edit_seek->text();
        if(!key.isEmpty()){
            QUrl url(search);

            QUrlQuery query;
            if(website==1){
                query.addQueryItem("vt","48991243");
                query.addQueryItem("keyword",key);
                query.addQueryItem("from_source","webtop_search");
                query.addQueryItem("spm_id_from","333.1007");
                query.addQueryItem("search_source","3");
            }else if(website==2){
                query.addQueryItem("w", key);
                query.addQueryItem("t", "song");
                query.addQueryItem("remoteplace", "txt.yqq.top");
                // query.addQueryItem("w", key);
                // query.addQueryItem("format", "json");
                // query.addQueryItem("p", "1");
                // query.addQueryItem("n", "10");
                // query.addQueryItem("cr", "1");
                // query.addQueryItem("ct", "24");
                // query.addQueryItem("new_json", "1");
                // query.addQueryItem("remoteplace", "txt.yqq.top");
            }else if(website==3){
                query.addQueryItem("keyword", QUrl::toPercentEncoding(key));
                query.addQueryItem("page", "1");
                query.addQueryItem("pagesize", "30");
                query.addQueryItem("platform", "WebFilter");
                query.addQueryItem("filter", "0");
                query.addQueryItem("iscorrection", "1");
                query.addQueryItem("privilege_filter", "0");
                query.addQueryItem("srcappid", "2919");
                query.addQueryItem("clientver", "20000");
                query.addQueryItem("clienttime", QString::number(QDateTime::currentMSecsSinceEpoch()));
            }

            url.setQuery(query);
            qDebug()<<"url"<<url;
            QNetworkRequest request(url);

            if(website==1){

                request.setRawHeader("User-Agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
                request.setRawHeader("Referer", "https://www.bilibili.com/");

            }else if(website==2){

                request.setRawHeader("User-Agent", "Mozilla/5.0 (iPhone; CPU iPhone OS 13_2_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/13.0.3 Mobile/15E148 Safari/604.1");
                request.setRawHeader("Referer", "https://y.qq.com/m/index.html");
                request.setRawHeader("Host", "c.y.qq.com");

            }else if(website==3){

                request.setRawHeader("User-Agent", "Mozilla/5.0 (Linux; Android 10; SM-G981B) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/80.0.3987.162 Mobile Safari/537.36");
                request.setRawHeader("Referer", "https://m.kugou.com/");
                request.setRawHeader("Host", "complexsearch.kugou.com");

            }

            manager->get(request);
        }
    });

    connect(manager,&QNetworkAccessManager::finished,this,[=](QNetworkReply* reply){
        if(reply->error()==QNetworkReply::NoError){
            QByteArray data=reply->readAll();
            qDebug()<<"data"<<data;
            reply->deleteLater();
            QList<QPair<QString,QString>> links_add_title=extractVideoLinks(data);
            qDebug()<<"links_add_title:"<<links_add_title;

            if(links_add_title.isEmpty()){
                QMessageBox::information(this,"提示","未获取到媒体资源！");
                return;
            }

            for(QPair<QString,QString>& pair:links_add_title){
                qDebug()<<"title"<<pair.second;
                QLabel* label=new QLabel(this);
                QString text=QString("<a href=\"%1\">%2</a>").arg(pair.first,pair.second);
                label->setText(text);
                label->setWordWrap(true);
                label->setOpenExternalLinks(true);
                label->setTextFormat(Qt::RichText);
                QVBoxLayout* vlayout=dynamic_cast<QVBoxLayout*>(ui->scrollAreaWidgetContents->layout());
                if(vlayout){
                    vlayout->insertWidget(vlayout->count()-1,label);
                }else{
                    qDebug()<<"类型转换失败";
                }
            }
        }else{
            qDebug()<<"reply->error()"<<reply->error();
            QMessageBox::warning(this,"警告","reply error:"+QString::number(reply->error()));
            reply->deleteLater();
        }
    });

    connect(ui->btn_clear,&QPushButton::clicked,this,[=](){
        ui->edit_seek->clear();
        const QList<QObject*> list=ui->scrollAreaWidgetContents->children();
        qDeleteAll(list);
        ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout);

        QLabel* label=new QLabel("来源于网络",this);
        label->setAlignment(Qt::AlignCenter);
        ui->scrollAreaWidgetContents->layout()->addWidget(label);
        ui->scrollAreaWidgetContents->layout()->addItem(new QSpacerItem(0,0,QSizePolicy::Ignored,QSizePolicy::Expanding));
    });

    connect(ui->edit_seek,&QLineEdit::returnPressed,this,[=](){
        emit ui->btn_seek->clicked();
    });
}

NetworkWidget::~NetworkWidget()
{
    delete ui;
}

QList<QPair<QString,QString>> NetworkWidget::extractVideoLinks(const QString &html) {
    QList<QPair<QString, QString>> videoData;

    // if(website == 2){
    //     QJsonParseError error;
    //     QJsonDocument doc = QJsonDocument::fromJson(html.toUtf8(), &error);
    //     if(error.error != QJsonParseError::NoError){
    //         qDebug() << "JSON解析错误:" << error.errorString();
    //         return videoData;
    //     }

    //     QJsonObject root = doc.object();
    //     QJsonObject dataObj = root["data"].toObject();
    //     QJsonObject songObj = dataObj["song"].toObject();
    //     QJsonArray list = songObj["list"].toArray();

    //     for(const QJsonValue &value : list){
    //         QJsonObject song = value.toObject();
    //         QString title = song["songname"].toString();
    //         QString mid = song["media_mid"].toString();

    //         // 构建歌曲详情页链接
    //         QString link = QString("https://y.qq.com/n/ryqq/songDetail/%1").arg(mid);

    //         // 获取歌手信息
    //         QJsonArray singers = song["singer"].toArray();
    //         QStringList singerList;
    //         for(const QJsonValue &singerVal : singers){
    //             singerList << singerVal.toObject()["name"].toString();
    //         }

    //         videoData.append(qMakePair(link, QString("%1 - %2").arg(title, singerList.join("/"))));
    //     }
    //     return videoData;
    // }


    static QRegularExpression regex_bilibili(R"(<a\s+[^>]*href="(//www\.bilibili\.com/video/[^"]+)\"[^>]*>\s*<h3[^>]*title="([^"]*)\"[^>]*>)");
    static QRegularExpression regex_qq_music(R"(<span class="songlist__songname_txt"><a title="([^"]+)\" href="(/n/ryqq/songDetail/[^"]+)\"[^>]*>.*?<span class="c_tx_highlight">([^<]+)</span>.*?</a></span>)");
    QRegularExpressionMatchIterator it;
    if(website==1){
        it=regex_bilibili.globalMatch(html);
    }else if(website==2){
        it=regex_qq_music.globalMatch(html);
    }

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString link = match.captured(1);
        QString title = match.captured(2);

        if (!link.isEmpty() && !title.isEmpty()) {
            videoData.append(qMakePair("https:" + link,title));
        }
    }
    return videoData;
}
