const LocalStrategy = require('passport-local').Strategy
const bcrypt = require('bcrypt')
const mysql = require('mysql');

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

function initialize(passport, getUserByEmail, getUserById) {
  var user
  const authenticateUser = async (email, password, done) => {
    
    //const rows = await db.query( 'SELECT * FROM users WHERE email = ?', [email]);

   // user = rows[0]
    //console.log(rows);
    db.query('SELECT * FROM users WHERE email = ?', [email], async(error, results) => {
      if(error) throw error;
      user = results[0]
    /*let sql = `SELECT * FROM users WHERE email = ${email}`;
    let query = db.query(sql, (err, result) => {
        //if(err) throw err;
        console.log(result);
    });*/
    
    if (user == null) {
      return done(null, false, { message: 'No user with that email' })
    }
    if (user.authorisation != 2580) {
      return done(null, false, { message: 'No admin rights' })
    }

    try {
      if (await bcrypt.compare(password, user.password)) {
        return done(null, user)
      } else {
        return done(null, false, { message: 'Password incorrect' })
      }
    } catch (e) {
      return done(e)
    }
  })
  }

  passport.use(new LocalStrategy({ usernameField: 'email' }, authenticateUser))
  passport.serializeUser((user, done) => done(null, user.id))
  passport.deserializeUser(function(id, done){
    db.query("SELECT * FROM users WHERE id = ? ", [id],
     function(err, rows){
      done(err, rows[0]);
     });
   });
}

module.exports = initialize