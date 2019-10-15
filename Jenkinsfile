// https://jenkins.io/doc/book/pipeline/jenkinsfile/
// Scripted pipeline (not declarative)
pipeline {
  options {
    buildDiscarder(logRotator(numToKeepStr: '10'))
  }

  stages {
    stage('Build & deliver') {
      agent { docker 'mauriciojost/arduino-ci:platformio-3.5.3-0.2.0' }
      stages {
        stage('Pull dependencies') {
          steps {
            script {
              sshagent(['bitbucket_key']) {
                wrap([$class: 'AnsiColorBuildWrapper', 'colorMapName': 'xterm']) {
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
