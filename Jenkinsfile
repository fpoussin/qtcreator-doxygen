pipeline {
  agent {
    docker {
      image 'fpoussin/jenkins:ubuntu-18.04-qtcreator-4.11'
    }

  }
  stages {
    stage('Prepare') {
      steps {
        sh '''git submodule sync
git submodule update --init'''
      }
    }

    stage('Build') {
      steps {
        sh '''mkdir build
cd build
export HOME=/tmp
qmake QTC_SOURCE=/qtcreator QTC_BUILD=/qtcreator USE_USER_DESTDIR=no ..
nice make -j $(nproc)'''
      }
    }

    stage('Artifacts') {
      steps {
        archiveArtifacts(artifacts: 'libDoxygen.so', caseSensitive: true, onlyIfSuccessful: true)
      }
    }

  }
}