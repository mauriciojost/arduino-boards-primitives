// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
// Use the snippet generator for more help: https://jenkins.martinenhome.com/job/botino-arduino/pipeline-syntax/
pipeline {
  triggers {
    pollSCM '* * * * *'
  }
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }

  stages {
    stage('Build & deliver') {
      agent { docker 'mauriciojost/arduino-ci:python-python_3.6-platformio-5.1.1-gcovr-4.1' }
      stages {

        stage('Update build refs') {
          steps {
            script {
              def libraryjson = readJSON file: 'library.json'
              def vers = libraryjson['version']
              def buildId = env.BUILD_ID
              currentBuild.displayName = "#$buildId - $vers"
            }
          }
        }

        stage('Pull dependencies') {
          steps {
            script {
              sshagent(['bitbucket_key']) {
                wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
                  sh 'git submodule update --init --recursive'
                  sh '.mavarduino/create_links'
                  sh 'export GIT_COMMITTER_NAME=jenkinsbot && export GIT_COMMITTER_EMAIL=mauriciojostx@gmail.com && set && ./pull_dependencies -p -l'
                }
              }
            }
          }
        }
        stage('Artifact') {
          steps {
            wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
              sh './upload -n esp8266 -p profiles/test.prof -e' // shared volume with docker container
              sh './upload -n esp32 -p profiles/test.prof -e' // shared volume with docker container
              sh './upload -n x86_64 -p profiles/test.prof'
            }
          }
        }
      }

    }
  }

  agent any

  post {  
    failure {  
      emailext body: "<b>[JENKINS] Failure</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "ERROR CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: true, compressLog: false;
    }  
    success {  
      emailext body: "<b>[JENKINS] Success</b>Project: ${env.JOB_NAME} <br>Build Number: ${env.BUILD_NUMBER} <br> Build URL: ${env.BUILD_URL}", from: '', mimeType: 'text/html', replyTo: '', subject: "SUCCESS CI: ${env.JOB_NAME}", to: "mauriciojostx@gmail.com", attachLog: false, compressLog: false;
    }  
    always {
      deleteDir()
    }
  }
}
