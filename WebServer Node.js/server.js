if (process.env.NODE_ENV !== 'production') {
  require('dotenv').config()
}

const express = require('express')
const app = express()
const bcrypt = require('bcrypt')
const passport = require('passport')
const flash = require('express-flash')
const session = require('express-session')
const methodOverride = require('method-override')
const mysql = require('mysql');
const expressLayouts = require('express-ejs-layouts')
var bodyParser = require('body-parser');

app.set('view engine', 'ejs')
app.set('views', __dirname + '/views')
app.set('layout', 'layouts/layout')
app.use(expressLayouts)
app.use(express.static('public'))
app.use(bodyParser.urlencoded({extended: true}));
app.set('port', process.env.PORT || 8890);


// Create connection
const db = mysql.createConnection({
    host     : 'localhost',
    user     : 'root',  
    password : '',  //db password
    database : '' //db name
});

// Connect
db.connect((err) => {
    if(err){
        throw err;
    }
    console.log('MySql Connected...');
});

const initializePassport = require('./passport-config')
initializePassport(
  passport,
  email => users.find(user => user.email === email),
  id => users.find(user => user.id === id)
)
const users = []

app.use(express.urlencoded({ extended: false }))
app.use(flash())
app.use(session({
  secret: process.env.SESSION_SECRET,
  resave: false,
  saveUninitialized: false
}))
app.use(passport.initialize())
app.use(passport.session())
app.use(methodOverride('_method'))

app.get('/bgh', function (req,res,next){
	res.render('watch', {	});
	const port = 8887;  //arduino port
	const host = '';    //arduino local ip
	const data = '828'; //code
	console.log('|-------------------------|Opening with watch.|-------------------------|');
	TCPConnect(host, port, data, res, false);
});

app.use(bodyParser.json());

app.get('/', checkAuthenticated, (req, res) => {
  res.render('index', { name: req.user.name })
})

app.post('/', checkAuthenticated, (req, res) => {
	console.log(req.body.gate);
if(req.body.gate === "OPENGARAGE")
{
	console.log('No!');
	res.send(JSON.stringify({ack: 706}));
}
if(req.body.gate === "OPENBACKGATEWHOLE")
{
	const port = 8887;  //arduino port
	const host = '';    //arduino local ip
	const data = '828'; //code
	TCPConnect(host, port, data, res, true);
}
if(req.body.gate === "OPENBACKGATEHALF")
{
	const port = 8887;  //arduino port
	const host = '';    //arduino local ip
	const data = '828'; //code
	TCPConnect(host, port, data, res, true);
}
})

app.get('/login', checkNotAuthenticated, (req, res) => {
  res.render('login')
})

app.post('/login', checkNotAuthenticated, passport.authenticate('local', {
  successRedirect: '/',
  failureRedirect: '/login',
  failureFlash: true
}))

app.get('/register', checkNotAuthenticated, (req, res) => {
  res.render('register')
})

app.get('/cameras', checkAuthenticated, (req, res) => {
  res.render('cameras')
})

app.get('/graph', checkAuthenticated, (req, res) => {
  res.render('graph')
})

app.get('/gates', checkAuthenticated, (req, res) => {
  res.render('gates')
})

app.post('/register', checkNotAuthenticated, async (req, res) => {
  try {
    const hashedPassword = await bcrypt.hash(req.body.password, 10)
    users.push({
      id: Date.now().toString(),
      name: req.body.name,
      email: req.body.email,
      password: hashedPassword
    })
    let post = {name:req.body.name, password:hashedPassword, email:req.body.email};
    let sql = 'INSERT INTO users SET ?';
    let query = db.query(sql, post, (err, result) => {
        if(err) throw err;
        //res.send('Post 1 added...');
    });
    res.redirect('/login')
  } catch {
    res.redirect('/register')
  }
})

app.delete('/logout', (req, res) => {
  req.logOut()
  res.redirect('/login')
})

function checkAuthenticated(req, res, next) {
  if (req.isAuthenticated()) {
    return next()
  }

  res.redirect('/login')
}

function checkNotAuthenticated(req, res, next) {
  if (req.isAuthenticated()) {
    return res.redirect('/')
  }
  next()
}

app.use(function (req, res){
	res.status(404);
	res.render('404');
});

app.use(function (err, req, res, next){
	console.error(err.stack);
	res.status(500);
	res.render('500');
});

app.listen(app.get('port'), function () {
	console.log('Express started on http://192.168.1.183:' + app.get('port'));
})

function TCPConnect(host, port, data, res, output) {
    const Net = require('net');
    var ack;
    const client = new Net.Socket();

    client.connect({ port: port, host: host }, function() {
      console.log('TCP connection established with the server.');
      var start = String.fromCharCode(2);
      var end = String.fromCharCode(3);
      data = start + data + end;
      client.write(data);
    });

    client.on('data', function(chunk) {
      console.log(`Data received from the server: ${chunk.toString()}.`);
      ack = chunk.toString();
      client.end();
    });
    
    client.on('end', function() {
      console.log('Requested an end to the TCP connection');
    });

    client.on('error', function() {
      console.log('Server dissonnected');
      console.log(ack)
      if(output)
      {
        res.send(JSON.stringify({ack: ack}));
      }
      
      return ack;
    });
}